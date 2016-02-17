#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 指定IP:port 来解析list文件
################################################################

import sys
from client import  srpp_parser
from client.ttypes import *
from thrift import Thrift
from sogou.conf import *
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TCompactProtocol
from utils.utils import *
import threading,simplejson
from urllib import unquote
import time,datetime,os,sys,random,re
reload(sys)
sys.setdefaultencoding('utf-8')

class com_parse:
    def __init__(self,host,list_file):
        self.conf=sogou_conf
        tmp=os.path.basename(list_file).split("_")
        self.tm_str=tmp[1]
        self.item_file_dir = self.conf["item_file_dir"]
        utils.mk_sure_dir(self.item_file_dir)
        self.host=host
        self.list_file = list_file
        tmp = self.host.split(":")
        transport = TSocket.TSocket(tmp[0], int(tmp[1]))
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)
        self.client = srpp_parser.Client(protocol)
        transport.open()
        print "ping %s %d %s" %(self.host,self.client.ping(),self.list_file)

    def parse_html(self):
        fp = open(self.list_file,"r")
        #query、province-city、time、remoteIP、html_file
        for line in fp:
            line = line.strip()
            item = line.split("\t")
            if len(item) < 5:
                continue
            query = item[0]
            html_file = item[-1]
            arg = parse_arg()
            arg.html_content = open(html_file,"r").read()
            item_file = self.item_file_dir.rstrip("/") +  "/" + self.tm_str +"/" + os.path.basename(html_file)[:-5] + ".item"
            if os.path.isfile(item_file):
                continue
            utils.mk_sure_dir(item_file)
            arg.extra = simplejson.dumps(dict({
                "send_dest":"root@10.141.49.52:" + item_file,

            }))
            arg.engine = "sogou"
            arg.ret_action = 2
            stime1 = time.time()
            ret_info = self.client.parseHtml(arg)
            stime2 = time.time()
            diff_t = stime2 - stime1
            print ">>>>>>>parse success ip[%s] time[%f] info[%s]" % (self.host,diff_t,str(ret_info))
        fp.close()
        return True

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print "USGAE: %s %s %s" % (sys.argv[0],"ip:port","list_file")
        sys.exit()
    host=sys.argv[1]
    if host not in sogou_conf["parse_host"]:
        print "invalid ip:port"
        sys.exit()
    list_file=sys.argv[2]
    if not os.path.isfile(list_file):
        print "file not exists %s" % list_file
        sys.exit()
    obj=com_parse(host,list_file)
    obj.parse_html()
