#!/usr/local/bin/python
# -*- coding: utf-8 -*-

import sys,os,time,simplejson
from down_thrift import srpp_task
from down_thrift.ttypes import *
from thrift.transport import TSocket  
from thrift.transport import TTransport  
from thrift.protocol import TBinaryProtocol  
from thrift.server import TServer
from Utils.utils import *
from Utils.action import *
reload(sys)
sys.setdefaultencoding('utf-8')


class srpp_task_handler:
    def __init__(self):
        self.config_file = "./parser_client.ini"
        self.arg = None
  
    def ping(self):
        return 0

    #在thrift里注册的方法
    def run_action(self, pa_arg):
        self.arg = pa_arg
        errno = 0
        errmsg = "success"
        try:
            self.__do_action__()
        except Exception,args:
            print args
            errno = 3
            errmsg = str(args)
        ret = dict({"errno":errno,"errmsg":errmsg})
        return simplejson.dumps(ret)

    #处理具体的操作的
    def __do_action__(self):
        task_id = self.arg.task_id
        obj = action(task_id,self.config_file)

        #生成种子文件
        obj.gen_seed_file()

        #启动下载
        obj.start_download()

        #启动解析
        obj.start_parse()

        #启动入库
        obj.import_db()
        return True
  
if __name__ == "__main__":
    if len(sys.argv) < 2:
        p = 9999
    elif sys.argv[1].isdigit():
        p = int(sys.argv[1])
    else:
        print "invalid parameter \n"
        sys.exit()
    handler = srpp_task_handler()
    processor = srpp_task.Processor(handler)
    transport = TSocket.TServerSocket(port=p)  
    tfactory = TTransport.TBufferedTransportFactory()  
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()  
    server = TServer.TForkingServer(processor, transport, tfactory, pfactory)
    print 'Starting the server...'  
    server.serve()  