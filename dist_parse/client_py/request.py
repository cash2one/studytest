#!/usr/local/bin/python
# -*- coding: utf-8 -*-

import sys,simplejson
from srpp_parser import  srpp_parser  
from srpp_parser.ttypes import *  
from thrift import Thrift  
from thrift.transport import TSocket  
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol  
from thrift.protocol import TCompactProtocol  

class parse_request:
    def __init__(self, ip, port):
        transport = TSocket.TSocket(ip, int(port))
        transport = TTransport.TBufferedTransport(transport)  
        protocol = TBinaryProtocol.TBinaryProtocol(transport)  
        self.client = srpp_parser.Client(protocol)  
        transport.open()
        self.host = ip + ":" + str(port)
        print "ping %s \t %d" % (self.host, self.client.ping())
        
    def run(self):
        arg = parse_arg()
        arg.html_content = open("/search/oss/new/application/snapshot/2014-08/2174_1/fffb581a8b9947f2ba57c6edd555be3c.html","r").read()
        arg.extra = simplejson.dumps(dict({
            "send_dest":"root@10.11.195.131:/search/liuyongshuai/client/abc.txt",
        }))
        arg.engine = "sogou"
        arg.ret_action = 2
        print "parseHtml %s %s" % (self.host ,self.client.parseHtml(arg))
        return True
          
if __name__ == '__main__':  
    hosts = list([
        "10.11.195.121:9090",
        
        "10.11.195.131:9090",
        "10.11.195.131:9091",
        
        "10.11.215.30:9090",
        
        "10.12.143.88:9090",
        "10.12.143.88:9091",
        
        "10.136.27.153:9090",
        "10.136.27.153:9091",
        
        "10.136.30.20:9091",
        "10.136.30.20:9092",
        
        "10.141.49.52:9090",
    ])
    for host in hosts:
        tmp = host.split(":")        
        req = parse_request(tmp[0],int(tmp[1]))
        req.run()
