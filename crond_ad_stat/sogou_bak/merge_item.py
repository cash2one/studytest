#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 合并item为一个文件
################################################################
import sys
from utils.utils import *
import threading,simplejson
from urllib import unquote
import time,datetime,os,sys,random,re
import fcntl
reload(sys)
sys.setdefaultencoding('utf-8')

class merge_item(threading.Thread):
    def __init__(self, config):
        threading.Thread.__init__(self)
        self.conf=config
        self.tm_str = time.strftime('%Y%m%d',time.localtime(utils.getThriDate()))
        self.ret_dir = self.conf["result_file_dir"].rstrip("/") + "/" + self.tm_str
        if not os.path.isdir(self.ret_dir):
            os.makedirs(self.ret_dir,0755)
        self.ret_fd = open(self.ret_dir + "/result","a")

    def run(self):
        while True:
            try:
                #解析完之后的list文件
                fileName = self.conf["mergeQueueObj"].get(True,5)
            except Exception,args:
                print "get mergeQueueObj error",args
                time.sleep(random.randint(1,5))
                continue
            if not fileName:
                continue
            bname = os.path.basename(fileName)
            print "[%s] mergeQueueObj getQueue %s" % \
                  (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),bname)
            fd = open(fileName,"r")
            for line in fd:
                self.__merge_item__(line)
            fd.close()

        self.ret_fd.close()
        return True

    #query、prov-city、time、ip、html_file
    def __merge_item__(self,line):
        tmp = line.strip()
        item = tmp.split("\t")
        if len(item) < 5:
            return False
        query = item[0]
        prov_city = item[1]
        visit_time = item[2]
        visit_ip = item[3]
        html_file = item[4]
        item_file = self.conf["item_file_dir"].rstrip("/") + "/" + self.tm_str +"/" + os.path.basename(html_file)[:-5] + ".item"
        if not os.path.isfile(item_file):
            return False
        final_ret = dict({
            "query":query,
            "time":visit_time,
            "ip":visit_ip,
            "prov_city":prov_city,
            "html_file":html_file
        })
        item_str = open(item_file,"r").read()
        parse_ret = item_str.split("\n")[0].strip()
        try:
            parse_ret = simplejson.loads(parse_ret,strict=False)
        except Exception,args:
            print args
            os.remove(item_file)
            return
        if "errno" in parse_ret:
            parse_ret = parse_ret["errmsg"]
        final_ret["ad_pzl_num"] = parse_ret["ad_pzl_num"]
        final_ret["ad_top_num"] = parse_ret["ad_top_num"]
        final_ret["ad_bottom_num"] = parse_ret["ad_bottom_num"]
        final_ret["list"] = parse_ret["list"]
        ostr = simplejson.dumps(final_ret)
        fcntl.flock(self.ret_fd, fcntl.LOCK_EX)
        self.ret_fd.write(ostr+"\n")
        self.ret_fd.flush()
        fcntl.flock(self.ret_fd,fcntl.LOCK_UN)
        return True