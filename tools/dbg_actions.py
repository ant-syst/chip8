import json

from dbg import dbg

class ActionError(Exception):

    def __init__(self, message):
        self.message = message

class Sender:

    def __init__(self, send_type):
        self.send_type = send_type

    def __call__(self):
        return {
            "type": self.send_type
        }

class Recver:

    def recv(self, client):
        rdata = client.recv(dbg.MSG_LEN).decode()
        data = json.loads(rdata)
        return data

    def __call__(self, client):
        return self.recv(client)

class Action:

    def __init__(self, client, sender, recver=None, checker=None):
        self.client = client
        self.sender = sender
        self.recver = recver
        self.checker = checker

    def __call__(self, *args, **kwargs):

        msg_send = self.sender(*args, **kwargs)
        self.client.send(json.dumps(msg_send).encode())

        if self.recver:
            msg_recv = self.recver(self.client)
            if self.checker and not self.checker(msg_send, msg_recv):
                raise ActionError(msg_recv)

            return msg_recv

class BkptSender(Sender):

    def __call__(self, addr):
        js = super().__call__()
        js["addr"] = addr
        return js

class BkptAddAction(Action):

    def __init__(self, client):
        bkpt_checker = lambda msg_send, msg_recv: msg_recv["addr"] == msg_send["addr"]
        super().__init__(client, BkptSender("bkpt_add"), Recver(), bkpt_checker)

class BkptRmAction(Action):

    def __init__(self, client):
        bkpt_checker = lambda msg_send, msg_recv: msg_recv["addr"] == msg_send["addr"]
        super().__init__(client, BkptSender("bkpt_rm"), Recver(), bkpt_checker)

class InfoBkptsAction(Action):

    def __init__(self, client):
        info_checker = lambda msg_send, msg_recv: msg_recv["type"] == "breakpoints"
        super().__init__(client, Sender("bkpt_info"), Recver(), info_checker)

class InfoCpuAction(Action):

    def __init__(self, client):
        info_checker = lambda msg_send, msg_recv: msg_recv["type"] == "cpu"
        super().__init__(client, Sender("cpu_info"), Recver(), info_checker)

class StopAction(Action):

    def __init__(self, client):
        super().__init__(client, Sender("stop"))

class ContAction(Action):

    def __init__(self, client):
        info_checker = lambda msg_send, msg_recv: msg_recv["value"] == "running"
        super().__init__(client, Sender("continue"), Recver(), info_checker)

    def is_async_msg(self, msg):
        return msg["type"] == "bkpt_triggered" or \
            (msg["type"] == "cpu_state" and msg["value"] == "stopped")

    def __call__(self):
        super().__call__()

        msg_recv = self.recver(self.client)
        if not self.is_async_msg(msg_recv):
            raise ActionError(msg_recv)
        return msg_recv

class Actions:

    def __init__(self, client):
        self.bkpt_add = BkptAddAction(client)
        self.bkpt_rm = BkptRmAction(client)
        self.info_bkpts = InfoBkptsAction(client)
        self.info_cpu = InfoCpuAction(client)
        self.cont = ContAction(client)
        self.stop = StopAction(client)
