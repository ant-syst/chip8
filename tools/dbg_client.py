#!/usr/bin/env python3

import readline
import re
import socket
import signal
import traceback
import sys
import os

from dbg import dbg
from dbg_actions import Actions, ActionError
from disassemble import Instruction

class Chip8:
    MEM_SIZE = 4096
    MEM_START = 0x200

class CommandNotFound(Exception):

    def __init__(self, command):
        self.command = command

    def __str__(self):
        return 'Undefined command: "%s". Try "help"' % self.command

class InvalidCommand(Exception):

    def __init__(self, message):
        self.message = message

class Command:

    def __init__(self, help, keyword, regexes):
        self.help = help
        self.keyword = keyword
        self.regexes = [re.compile(regex) for regex in regexes]

    def lookup_regex(self, text):
        for regex in self.regexes:
            match = regex.match(text)
            if match:
                return match
        return None

    def is_command(self, text):
        return self.lookup_regex(text) != None

    def complete(self, text):
        if self.keyword.startswith(text):
            return [self.keyword]
        return []

class DisassembleCommand(Command):

    def __init__(self, actions):
        self.actions = actions
        keyword = "disassemble"
        regex = r'^%s$' % keyword
        h = "disassemble -- Disassemble the current instruction."
        super().__init__(h, keyword, [regex])

    def __call__(self, text):
        infos = self.actions.info_cpu()
        it = infos["value"]["it"]
        addr1 = infos["value"]["pc"]
        addr2 = addr1 + 1
        byte1 = it & 0xFF
        byte2 = it >> 8 & 0xFF
        print(Instruction(addr1, byte1, addr2, byte2, False))

class NextCommand(Command):

    def __init__(self, actions):
        self.actions = actions
        keyword = "next"
        regex = r'^%s$' % keyword
        h = "next -- Step program."
        super().__init__(h, keyword, [regex])

    def __call__(self, text):
        infos = self.actions.info_bkpts()
        for bkpt_addr in range(Chip8.MEM_START, Chip8.MEM_SIZE - Chip8.MEM_START):
            if bkpt_addr not in infos["value"]:
                self.actions.bkpt_add(bkpt_addr)

        self.actions.cont()

        for bkpt_addr in range(Chip8.MEM_START, Chip8.MEM_SIZE - Chip8.MEM_START):
            if bkpt_addr not in infos["value"]:
                self.actions.bkpt_rm(bkpt_addr)

class HelpCommand(Command):

    def __init__(self, cmds):
        keyword = "help"
        regex = r'^%s$' % keyword
        h = "help -- Print list of commands."
        super().__init__(h, keyword, [regex])
        self.cmds = cmds

    def __call__(self, text):
        for cmd in self.cmds:
            print(cmd.help)

class ExitCommand(Command):

    def __init__(self):
        keyword = "exit"
        regex = r'^%s$' % keyword
        h = "exit -- Exit debugger."
        super().__init__(h, keyword, [regex])

    def __call__(self, text):
        raise SystemExit()

class ActionCommand(Command):

    def __init__(self, help, keyword, regexes, action):
        super().__init__(help, keyword, regexes)
        self.action = action

    def call(self, text):
        match = self.lookup_regex(text)
        return self.action(*match.groups())

    def __call__(self, text):
        res = self.call(text)
        self.post(res)

    def post(self, msg_recv):
        pass

class ContCommand(ActionCommand):

    def __init__(self, actions):
        self.actions = actions
        self.calling = False
        keyword = "cont"
        regexes = [r'^%s$' % keyword, r'^c$']
        h = "cont|c -- Continue program being debugged, after signal or breakpoint."
        super().__init__(h, keyword, regexes, actions.cont)

    def __call__(self, text):
        self.calling = True
        try:
            return super().__call__(text)
        finally:
            self.calling = False

    def post(self, msg_recv):
        if msg_recv["type"] == "cpu_state" and msg_recv["value"] == "stopped":
            print("Program received signal SIGINT, Interrupt.")
        elif msg_recv["type"] == "bkpt_triggered":
            addr = msg_recv["addr"]
            print("Breakpoint at %d:%#x" % (addr, addr))

    def sigint(self):
        if self.calling:
            self.actions.stop()

class BkptCommand(ActionCommand):

    def __init__(self, help, keyword, action):

        regexes = [regex % keyword for regex in [r'%s 0x([0-9a-fA-F]+)$', r'%s ([0-9]+)$']]
        self.bases = [16, 10]
        super().__init__(help, keyword, regexes, action)

    def lookup_regex(self, text):
        for base, regex in zip(self.bases, self.regexes):
            match = regex.match(text)
            if match:
                return (base, match)

    def call(self, text):
        base, match = self.lookup_regex(text)
        return self.action  (int(match.group(1), base))

class BkptAddCommand(BkptCommand):

    def __init__(self, actions):
        keyword = "break"
        h = "break -- Set breakpoint at specified location"
        super().__init__(h, keyword, actions.bkpt_add)

    def post(self, msg_recv):

        bkpt_addr = msg_recv["addr"]

        if msg_recv["type"] == "bkpt_added":
            print("Breakpoint at %d:%#x" % (bkpt_addr, bkpt_addr))
        elif msg_recv["type"] == "error":
            if msg_recv["value"] == "breakpoint_exists":
                print("Breakpoint already set at %d:%#x" % (bkpt_addr, bkpt_addr))
            elif msg_recv["value"] == "invalid_address":
                print("Invalid address at %d:%#x" % (bkpt_addr, bkpt_addr))

class BkptRmCommand(BkptCommand):

    def __init__(self, actions):
        keyword = "delete breakpoints"
        h = "delete breakpoint -- Delete a breakpoint at specified location"
        super().__init__(h, keyword, actions.bkpt_rm)

    def post(self, msg_recv):

        bkpt_addr = msg_recv["addr"]

        if msg_recv["type"] == "bkpt_removed":
            print("Breakpoint removed at %d:%#x" % (bkpt_addr, bkpt_addr))
        elif msg_recv["type"] == "error":
            if msg_recv["value"] == "breakpoint_not_found":
                print("No breakpoint at %d:%#x" % (bkpt_addr, bkpt_addr))
            elif msg_recv["value"] == "invalid_address":
                print("Invalid address at %d:%#x" % (bkpt_addr, bkpt_addr))


class InfoBreakpointsCommand(ActionCommand):

    def __init__(self, actions):
        keyword = "info breakpoints"
        regex = r'^%s$' % keyword
        h = "info breakpoints -- List of all breakpoints"
        super().__init__(h, keyword, [regex], actions.info_bkpts)

    def post(self, msg_recv):
        if len(msg_recv["value"]) == 0:
            print("No breakpoints")
        else:
            print("Address")
            for bkpt_addr in msg_recv["value"]:
                print("%-4d:%#-4x" % (bkpt_addr, bkpt_addr))

class InfoCpuCommand(ActionCommand):

    def __init__(self, actions):
        keyword = "info cpu"
        regex = r'^%s$' % keyword
        h = "info cpu -- List of all cpu registers and their contents"
        super().__init__(h, keyword, [regex], actions.info_cpu)

    def post(self, msg_recv):
        for key, value in msg_recv["value"].items():
            if key == "v":
                print("v", end="\t")
                for i, j in enumerate(value):
                    print("v%X " % i, end=" ")
                print()
                print(" ", end="\t")
                for i, j in enumerate(value):
                    print("%-3d" % j, end=" ")
                print()
            else:
                print(key, end="\t")
                print(value)

class Commands:

    def __init__(self, actions):
        self.cmds = [BkptRmCommand(actions), BkptAddCommand(actions), InfoBreakpointsCommand(actions), \
                    InfoCpuCommand(actions), ContCommand(actions), HelpCommand(self), \
                    NextCommand(actions), DisassembleCommand(actions), ExitCommand()]

    def __iter__(self):
        for cmd in self.cmds:
            yield cmd

    def completer(self):
        def complete(text, state):
            for cmd in self.cmds:
                cmpls = cmd.complete(text)
                for cmpl in cmpls:
                    if state == 0:
                        return cmpl
                    else:
                        state -= 1
            return None
        return complete

    def lookup(self, name):
        cmds = [cmd for cmd in self.cmds if cmd.is_command(name)]
        if len(cmds) != 1:
            raise CommandNotFound(name)
        return cmds[0]

class Interpreter:

    def connect(self):
        client = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
        client.connect(dbg.SOCK_PATH)
        return client

    def run(self):

        with self.connect() as client:

            commands = Commands(Actions(client))

            readline.set_completer(commands.completer())
            readline.set_completer_delims("\t")
            readline.parse_and_bind("tab: complete")

            def sigint_handler(signum, frame):
                commands.lookup("cont").sigint()
            signal.signal(signal.SIGINT, sigint_handler)

            while True:
                line = input("(dbg): ").strip()
                if line != "":
                    readline.add_history(line)
                try:
                    cmd = commands.lookup(line)
                    msg = cmd(line)

                except CommandNotFound as e:
                    if e.command != "":
                        print(e)
                except ActionError as e:
                    print("ActionError")
                    print(e.message)
                    raise e

if __name__ == '__main__':
    interpreter = Interpreter()
    interpreter.run()
