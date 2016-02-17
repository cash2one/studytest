#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 解析后的结果，合并成一个文件,list_20140723_04_sjs
################################################################


import sys
from sogou.conf import *
from utils.utils import *
import simplejson
import fcntl
reload(sys)
sys.setdefaultencoding('utf-8')

class com_merge:
    def __init__(self,list_file):
        tmp=os.path.basename(list_file).split("_")
        if len(tmp) != 4 or tmp[0] != "list":
            print "invalid list file"
            sys.exit()
        self.conf=sogou_conf
        self.item_file_dir = self.conf["item_file_dir"]
        self.tm_str=tmp[1]
        self.list_file=list_file
        ret_dir = self.conf["result_file_dir"].rstrip("/") + "/" + self.tm_str
        if not os.path.isdir(ret_dir):
            os.makedirs(ret_dir,0755)
        self.ret_fd = open(ret_dir + "/result","a")

    def do_merge(self):
        fd = open(self.list_file,"r")
        for line in fd:
            self.__merge_item__(line)
        fd.close()
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
        item_file = self.item_file_dir.rstrip("/") +  "/" + self.tm_str +"/" + os.path.basename(html_file)[:-5] + ".item"
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

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "USGAE: %s %s" % (sys.argv[0],"list_file")
        sys.exit()
    list_file=sys.argv[1]
    if not os.path.isfile(list_file):
        print "file not exists %s" % list_file
        sys.exit()
    obj=com_merge(list_file)
    obj.do_merge()