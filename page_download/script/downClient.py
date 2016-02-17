#!/usr/local/bin/python
# -*- coding: utf-8 -*-

import sys,simplejson
from down_thrift import  srpp_task
from down_thrift.ttypes import *
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
reload(sys)
sys.setdefaultencoding('utf-8')

class sendDownRequest:
    def __init__(self, ip, port):
        self.transport = TSocket.TSocket(ip, int(port))
        self.transport = TTransport.TBufferedTransport(self.transport)
        protocol = TBinaryProtocol.TBinaryProtocol(self.transport)
        self.client = srpp_task.Client(protocol)
        self.transport.open()
        self.host = ip + ":" + str(port)
        print "ping %s \t %d" % (self.host, self.client.ping())

    def run(self):
        arg = task_arg()
        arg.task_id = 2229
        print "parseHtml %s %s" % (self.host ,self.client.run_action(arg))
        self.transport.close()
        return True

if __name__ == '__main__':
    req = sendDownRequest("10.136.30.20",34567)
    req.run()