#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 解析线程
################################################################
import sys
from client import  srpp_parser
from client.ttypes import *
from thrift import Thrift
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

class sogou_sendParse(threading.Thread):
    def __init__(self, config, num):
        threading.Thread.__init__(self)
        self.conf=config
        self.tm_str = time.strftime('%Y%m%d',time.localtime(utils.getThriDate()))
        self.item_file_dir = self.conf["item_file_dir"]
        utils.mk_sure_dir(self.item_file_dir)
        self.host=self.conf["parse_host"][num]
        tmp = self.host.split(":")
        transport = TSocket.TSocket(tmp[0], int(tmp[1]))
        transport = TTransport.TBufferedTransport(transport)
        protocol = TBinaryProtocol.TBinaryProtocol(transport)
        self.client = srpp_parser.Client(protocol)
        transport.open()
        print "ping %s %d" %(self.host,self.client.ping())

    def run(self):
        while True:
            try:
                #list文件的绝对路径
                fileName = self.conf["parseQueueObj"].get(True,5)
            except Exception,args:
                print "get parseQueueObj error",args
                time.sleep(random.randint(1,5))
                continue
            if not fileName:
                continue
            bname = os.path.basename(fileName)
            print "[%s] getQueue %s" % (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),bname)

            if not os.path.isfile(fileName):
                continue
            #抽取html文件
            self.__parse_html__( fileName)
            try:
                self.conf["mergeQueueObj"].put(fileName,5)
            except Exception,args:
                print "add mergeQueueObj err",args
        return True

    def __parse_html__(self, list_file):
        fp = open(list_file,"r")
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
            arg.query = query
            item_file = self.item_file_dir.rstrip("/") +  "/" + self.tm_str +"/" + os.path.basename(html_file)[:-5] + ".item"
            if os.path.isfile(item_file):
                continue
            utils.mk_sure_dir(item_file)
            arg.send_dir = "root@10.141.49.52:" + item_file
            arg.engine = "sogou"
            arg.extra = line
            arg.need_return = 0
            stime1 = time.time()
            ret_info = self.client.parseHtml(arg)
            stime2 = time.time()
            diff_t = stime2 - stime1
            print ">>>>>>>parse success ip[%s] time[%f] info[%s]" % (self.host,diff_t,str(ret_info))
        fp.close()
        return True