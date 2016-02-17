#!/usr/local/bin/python
# -*- coding: utf-8 -*-

import sys,os,time,simplejson
from srpp_parser import srpp_parser
from srpp_parser.ttypes import *  
from thrift.transport import TSocket  
from thrift.transport import TTransport  
from thrift.protocol import TBinaryProtocol  
from thrift.server import TServer  
from webpage_parser import WebPageParser
from webpage_config import WebPageConfig
from conf import engine_conf,parse_conf
from utils.utils import *
from conf import *
from utils.down import down
from utils.ret_act import *
import logging
import logging.handlers
reload(sys)
sys.setdefaultencoding('utf-8')


class srpp_parser_handler:
    def __init__(self,proxy):
        self.data_dir = parse_conf["tmp_dir"]
        if not os.path.isdir(self.data_dir):
            os.makedirs(self.data_dir,0755)
        log_dir = parse_conf["logs_dir"]
        if not os.path.isdir(log_dir):
            os.makedirs(log_dir,0755)

        #日志设置
        self.tm_str = time.strftime('%Y%m%d',time.localtime(time.time()))
        log_file = log_dir.rstrip("/") + "/" + self.tm_str + ".log"
        self.logger = logging.getLogger(os.path.basename(__file__))
        self.logger.setLevel(logging.DEBUG)
        formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
        file_handler = logging.handlers.TimedRotatingFileHandler(log_file, when="D", backupCount=7, encoding="utf-8")
        file_handler.setFormatter(formatter)
        self.logger.addHandler(file_handler)

        #解析器配置
        self.config = dict()
        self.parser = WebPageParser(self.logger)

        #如果要下载的话
        self.download = down(proxy)

        #传过来的参数
        self.arg = None
  
    def ping(self):
        return 0

    ####################################################################
    # 在thrift里注册的方法
    # @return
    # {
    #    "errno":0 , #0表示成功，否则表示错误，其中，100表示被百度封了
    #    "errmsg":"", #成功时表示返回的数据，否则表示错误信息
    # }
    ####################################################################
    def parseHtml(self, pa_arg):
        self.arg = pa_arg
        ret = dict()
        try:
            item = self.__parse_action__()
            ret["errno"] = 0
            ret["errmsg"] = item
        except Exception,args:
            ret["errno"] = 2
            ret["errmsg"] = str(args)
        return simplejson.dumps(ret)

    ###################################################
    # 实际解析逻辑，有可能会抛异常
    ###################################################
    def __parse_action__(self):
        html_content = self.arg.html_content
        extra = self.arg.extra
        engine = self.arg.engine
        ret_action = int(self.arg.ret_action)

        #校验参数
        if len(html_content) <= 0: #若html为空，则extra里必定有url，直接下载即可
            edata = simplejson.loads(extra.strip())
            if not edata.has_key("url"):
                raise Exception("empty html string")
            else:
                html_content = self.download.getHtml(edata["url"])
                if len(html_content) <= 0:
                    raise Exception("download url %s failed" % edata["url"])
        if engine not in engine_conf:
            raise Exception("invalid engine %s" % engine)

        #实例化解析的类
        if engine not in self.config:
            self.config[engine] = WebPageConfig(self.logger)
            if self.config[engine].ParseFile(engine_conf[engine]) != 0:
                raise Exception("parse config file failed %s\n" % engine_conf[engine])

        #开始解析
        self.parser.Init(self.config[engine])
        self.parser.Reset()
        self.parser.ParseHTML(html_content)
        self.parser.RedirectUrl()

        ret = dict({
            "ad_pzl_num":self.parser.page_info.ad_pzl_num,
            "ad_top_num":self.parser.page_info.ad_top_num,
            "ad_bottom_num":self.parser.page_info.ad_bottom_num,
            "list":list(),
        })
        for index,result in enumerate(self.parser.page_element_list):
            if result.search_result == None:
                continue
            item = dict()
            item["rank"] = result.rank
            item["url"] = result.search_result.url.encode('utf-8','ignore')
            item["raw_url"] = result.search_result.raw_url.encode('utf-8','ignore')
            item["title"] = result.search_result.title.encode('utf-8','ignore')
            item["ad_type"] = result.ad_type
            item["docid"] = result.search_result.docid
            ret["list"].append(item)

        #判断之后的操作
        if ret_action == 0: #直接返回结果
            return ret
        elif ret_action == 1: #入库操作
            return ret_act.insert_db_ret(extra,**ret)
        elif ret_action == 2: #将解析的结果rsync到指定位置
            return ret_act.rsync_ret(extra,**ret)
        elif ret_action == 3: #将解析的结果及源html文件都往指定地址推一份
            return ret_act.rsync_ret_diff(html_content, extra, **ret)
        elif ret_action == 4: #仅回传html文件，解析的结果以json串形式返回
            return ret_act.rsync_ret_diff_diff(html_content, extra, **ret)
        elif ret_action == 5:#结果直接入库，html文件则回传一份
            ret_act.insert_db_ret(extra,**ret)
            ret_act.rsync_ret_diff_diff(html_content, extra, **ret)
            return "success"
        else: #当不知道怎么操作时就直接返回
            return ret
  
if __name__ == "__main__":
    if len(sys.argv) < 2:
        p = 9090
    elif sys.argv[1].isdigit():
        p = int(sys.argv[1])
    else:
        print "invalid parameter \n"
        sys.exit()

    proxy_host = ""
    if len(sys.argv) >= 3:
        proxy_host = sys.argv[2]
    handler = srpp_parser_handler(proxy_host.strip())
    processor = srpp_parser.Processor(handler)  
    transport = TSocket.TServerSocket(port=p)  
    tfactory = TTransport.TBufferedTransportFactory()  
    pfactory = TBinaryProtocol.TBinaryProtocolFactory()  
    server = TServer.TForkingServer(processor, transport, tfactory, pfactory)
    print 'Starting the server...'  
    server.serve()  