#!/usr/local/bin/python
# -*- coding: utf-8 -*-

#####################################################
# 监控线程，主要是读取要处理的日志文件，扔到队列里去
#####################################################

from utils.utils import *
import threading
import os,sys,time,random
reload(sys)
sys.setdefaultencoding('utf-8')

class sogou_monitor(threading.Thread):
    def __init__(self, config):
        threading.Thread.__init__(self)
        self.conf = config
        self.done_set = set()

    def run(self):
        self.__setQueue()

    ###########################################################################
    #开始往队列里添加日志文件
    ###########################################################################
    def __setQueue(self):
        #过滤掉已经处理完的日志文件
        tmStr = time.strftime('%Y%m%d',time.localtime(utils.getThriDate()))
        done_log = self.conf["done_log_file"] % tmStr
        fp = open(done_log,"r")
        for line in fp:
            self.done_set.add(line.strip())
        fp.close()
        #开始取待处理文件列表
        tmpList = os.popen("/bin/ls -1rt " + self.conf["ad_log_data_dir"] + " | grep utf8|grep -v tmp|grep -v done").readlines()
        for f in tmpList:
            log_file = f.strip()
            if len(log_file) <= 0:
                continue
            #不是以utf8结尾及隐藏文件，跳过
            if not log_file.endswith("utf8") or log_file.startswith("."):
                continue
            #日志文件的绝对路径
            log_file = self.conf["ad_log_data_dir"] + "/" + log_file
            if log_file in self.done_set:
                continue
            #队列满了，就等会
            while self.conf["extractQueueObj"].full():
                time.sleep(random.randint(10,30))
                continue
            #塞到队列里去，有可能会出错抛出异常
            try:
                self.conf["extractQueueObj"].put(log_file,True)
                self.done_set.add(log_file)
            except Exception,args:
                print args
                time.sleep(random.randint(1,5))
                continue
            print "[%s] addQueue %s" % (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),os.path.basename(log_file))

        return True