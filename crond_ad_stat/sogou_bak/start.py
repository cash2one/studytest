#!/usr/local/bin/python
# -*- coding: utf-8 -*-

from conf import *
from utils.utils import *
from monitor import *
from extract import  *
from sendParse import  *
from merge_item import *
reload(sys)
sys.setdefaultencoding('utf-8')

class sogou_start:
    def __init__(self):
        self.conf = sogou_conf
        self.tm_str = time.strftime('%Y%m%d',time.localtime(utils.getThriDate()))
        pass

    def start(self):
        done_log = self.conf["done_log_file"] % self.tm_str
        if not os.path.isfile(done_log):
            open(done_log,"w").close()

        #启动monitor线程
        monitor = sogou_monitor(self.conf)
        monitor.start()

        #启动抽取线程
        for no in range(self.conf["parseThreadNum"]):
            obj = sogou_extract(self.conf)
            obj.start()

        #启动解析发送线程
        for num,host in enumerate(self.conf["parse_host"]):
            obj = sogou_sendParse(self.conf,num)
            obj.start()

        #启动item合并线程
        for i in range(5):
            obj = merge_item(self.conf)
            obj.start()

        return True