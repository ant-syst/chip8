#!/usr/bin/env python3

import termcolor
import argparse
import subprocess
import re
import itertools

from termcolor import colored
from itertools import tee
from hw import HW

def pairwise(iterable):
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)

class ColoredColor:

    class RegexTokenColor:

        def __init__(self, color, pattern):
            self.color = color
            self.regex = re.compile(pattern)

        def match(self, token):
            return self.regex.match(token)

    def __init__(self):
        self.tokens_colors = []

        words = ["jump", "call", "return", "skip", "if", "sprite", \
                "at", "dw", "store", "load", "bcd", "timer", "stimer", \
                "==", "!=", "\+", "<-", "and", "or", "xor"]

        for word in words:
            self.tokens_colors.append(self.RegexTokenColor("magenta", "^" + word + ".*"))

        self.tokens_colors.append(self.RegexTokenColor("green", "I"))
        self.tokens_colors.append(self.RegexTokenColor("green", r"^v[A-F0-9]$"))
        self.tokens_colors.append(self.RegexTokenColor("green", r"^v[A-F0-9]:v[A-F0-9]$"))

        self.tokens_colors.append(self.RegexTokenColor("cyan", r"^0x[\dA-Fa-f]+$"))
        self.tokens_colors.append(self.RegexTokenColor("cyan", r"^[0-9]+$"))


    def lookup_token(self, token):
        for token_color in self.tokens_colors:
            if token_color.match(token):
                return token_color

        return None

    def colored_code(self, code):

        colored_code = ""
        has_comment = False

        for i, token in enumerate(code.split(" ")):
            token_color = self.lookup_token(token)

            if has_comment:
                colored_code += colored(token, "white", attrs=["bold", "dark"])
            else:
                has_comment = token.startswith(";")

                if token_color:
                    colored_code += colored(token, token_color.color, attrs=["bold"])
                else:
                    colored_code += token

            colored_code += " "

        return colored_code.rstrip()

class Column:

    def __init__(self):
        self.lines = []

    def append(self, line):
        self.lines.append(line)

    @property
    def max_len(self):
        return max([len(line) for line in self.lines])

class MultiColumns:

    def __init__(self, columns, sep):
        self.columns = columns
        self.sep = sep

    def print_columns(self):

        lines = [column.lines for column in self.columns]

        for line in itertools.zip_longest(*lines):
            for i, cell in enumerate(line):
                cell = "" if cell is None else cell

                print("%s" % cell, end="")
                print((self.columns[i].max_len - len(cell)) * " ", end="")
                print(self.sep, end="")
            print()

class Sprite:

    def __init__(self, addr, byte):
        self.addr = addr
        self.byte = byte

    def __str__(self):

        buff = "%#4x: %#x" % (self.addr, self.byte)

        for i in range(8, 0, -1):
            if self.byte >> i & 1 == 1:
                buff += "\u2588"
            else:
                buff += " "

        return buff

    def __len__(self):
        return len(self.__str__())

class Comment:

    cc = ColoredColor()

    def __init__(self, addr, text):
        self.addr = addr
        self.text = text

    def __str__(self):
        return Comment.cc.colored_code(self.text)

    def __len__(self):
        return len(self.text)

class Instruction:

    cc = ColoredColor()

    def __init__(self, addr1, byte1, addr2, byte2, is_label):
        self.addr1 = addr1
        self.byte1 = byte1

        self.addr2 = addr2
        self.byte2 = byte2

        self.is_label = is_label

        it = self.little_endian_it()

        self.decoded_it = subprocess.check_output(["./bin/disassemble.elf", "%x" % it]).decode().rstrip()

    @property
    def addr(self):
        return self.addr1

    def little_endian_it(self):
        return self.byte1 << 8 | self.byte2

    def __str__(self):

        addr = "%#4x" % self.addr1
        if self.is_label:
            addr = colored(addr, "red", attrs=["bold"])

        hexa = colored("%#6x" % self.little_endian_it(), "blue", attrs=["bold"])
        code = Instruction.cc.colored_code(self.decoded_it)
        return "%s: %s %s" % (addr, hexa, code)

    def __len__(self):
        return len("%#4x: %#6x %s" % (self.addr1, self.little_endian_it(), \
                                self.decoded_it))

class Hexadeciml:

    def __init__(self, addr, byte):
        self.addr = addr
        self.byte = byte

    def __str__(self):
        return "%#4x: %#x" % (self.addr, self.byte)

    def __len__(self):
        return len(self.__str__())

class ROM:

    def __init__(self, binary):
        self.start = HW.MEM_START
        self.mem_size = HW.MEM_SIZE

        if len(binary) > (self.mem_size - self.start):
            raise ValueError("Program size too large")

        self._memory = bytes([0 for e in range(self.start)]) + binary

    def addresses(self, skip_last=False):

        end = len(self._memory)
        if skip_last:
            end -= 1

        for address in range(self.start, end):
            yield address

    def memory(self, skip_last=False):

        end = len(self._memory)
        if skip_last:
            end -= 1

        for byte in self._memory[self.start:end]:
            yield byte

    def addr_mem(self):

        for addr, byte in zip(self.addresses(), self._memory[self.start:]):
            yield (addr, byte)

class Disassembler:

    def __init__(self, rom, labels_addrs=None, its_addr=None, data_addr=None):
        self.rom = rom
        self.labels_addrs = [] if labels_addrs is None else list(labels_addrs)
        self.its_addr = [] if its_addr is None else list(its_addr)
        self.data_addr = [] if data_addr is None else list(data_addr)

    def disassemble(self):

        addr_mem = itertools.chain(self.rom.addr_mem(), [(None, None)])

        for (addr1, byte1), (addr2, byte2) in iter(pairwise(addr_mem)):
            if addr1 in self.its_addr:
                if byte2 is not None:
                    yield Instruction(addr1, byte1, addr2, byte2, addr1 in self.labels_addrs)
            elif addr1 in self.data_addr:
                yield Sprite(addr1, byte1)
            else:
                yield Hexadeciml(addr1, byte1)

def disassemble_instructions(args):

    col1 = Column()
    col2 = Column()
    rom = ROM(args.rom.read())
    disassembler = Disassembler(rom, its_addr=rom.addresses(skip_last=True))
    for i, obj in enumerate(disassembler.disassemble()):
        if i % 2 == 0:
            col1.append(obj)
        else:
            col2.append(obj)

    mc = MultiColumns([col1, col2], "\t\t")
    mc.print_columns()

def disassemble_sprites(args):

    rom = ROM(args.rom.read())
    disassembler = Disassembler(rom, data_addr=rom.addresses())
    for obj in disassembler.disassemble():
        print(obj)

def disassemble_infos(args):

    col1 = Column()
    col2 = Column()
    rom = ROM(args.rom.read())

    its_addrs = []
    if args.its_addrs_path:
        its_addrs = [int(i.rstrip(), 16) for i in args.its_addrs_path.readlines()]

    data_addrs = []
    if args.data_addrs_path:
        data_addrs = [int(i.rstrip(), 16) for i in args.data_addrs_path.readlines()]

    labels_addrs = []
    if args.labels_path:
        labels_addrs = [int(i.rstrip(), 16) for i in args.labels_path.readlines()]

    comments = {}
    if args.comments_path:
        # TODO make function
        regex = re.compile(r"^(0x[\dA-Fa-f]+)(.+)$")
        for line in args.comments_path.readlines():
            line = line.strip()
            matcher = regex.match(line)
            if matcher:
                comment = Comment(int(matcher.group(1), 16), matcher.group(2).strip())
                comments[comment.addr] = comment

    for addr in rom.addresses():
        if addr not in its_addrs and addr not in data_addrs:
            if args.process_remaining_adresses_as_instructions:
                its_addrs.append(addr)
            elif args.process_remaining_adresses_as_sprites:
                data_addrs.append(addr)

    disassembler = Disassembler(rom, labels_addrs=labels_addrs, data_addr=data_addrs, its_addr=its_addrs)
    for obj in disassembler.disassemble():
        if type(obj) is not Hexadeciml:
            col1.append(obj)
            comment = comments.get(obj.addr)
            if comment:
                col2.append(comment)
            else:
                col2.append("")

    mc = MultiColumns([col1, col2], "\t")
    mc.print_columns()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Disassemble chip8 programms.")
    h = "Chip8 rom path to disassemble"
    parser.add_argument("--rom", type=argparse.FileType('rb'), required=True, help=h)

    sp = parser.add_subparsers()

    h = "Disassemble Chip8 rom in two columns, the left for paired addresses instructions, " + \
        "the right for impair addresses instructions"
    sp_disassemble_instructions = sp.add_parser("instructions", help=h)
    sp_disassemble_instructions.set_defaults(func=disassemble_instructions)

    h = "Disassemble the whole Chip8 rom as sprites"
    sp_print_sprites = sp.add_parser("sprites", help=h)
    sp_print_sprites.set_defaults(func=disassemble_sprites)

    h = "Disassemble using additional informations to get instructions/sprites addresses."
    sp_infos = sp.add_parser("infos", help=h)
    sp_infos.add_argument("--its-addrs-path", type=argparse.FileType('r'))
    sp_infos.add_argument("--data-addrs-path", type=argparse.FileType('r'))
    sp_infos.add_argument("--labels-path", type=argparse.FileType('r'))
    sp_infos.add_argument("--comments-path", type=argparse.FileType('r'))
    sp_infos.set_defaults(func=disassemble_infos)
    group = sp_infos.add_mutually_exclusive_group()
    group.add_argument("--process-remaining-adresses-as-instructions", default=False, action="store_true")
    group.add_argument("--process-remaining-adresses-as-sprites", default=False, action="store_true")

    #
    # ITERATOR
    #
    # def rom.memory():
    #   for addr1, addr2, byte in rom.memory
    #       if addr1 in its_addr:
    # it;       yield new It(addr1, addr2)
    #           skip next addr
    #           remove addr1 from its_addr
    #       elif addr1 in data_addr:
    # sprite    yield new Sprite(addr1)
    #           remove addr1 from data_addr
    #       else:
    #           if threat-other-adresses-as-instructions
    #               goto it
    #           else
    #               goto sprite
    #

    args = parser.parse_args()
    args.func(args)
