#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 指定以utf8结尾的文件，提取里面的html文件，文件名如“04_sjs_sogouad.log.2014-07-23.utf8”
################################################################


import sys
from sogou.conf import *
import simplejson
from urllib import unquote
import time,datetime,os,sys,random,re
import IP
import hashlib
reload(sys)
sys.setdefaultencoding('utf-8')


class com_extract:
    def __init__(self,log_file):
        self.conf = sogou_conf
        bname = os.path.basename(log_file)
        tmp=bname.split(".")
        tmp=tmp[2]
        self.tm_str = tmp.replace("-","")
        suffix = bname[0:bname.find("sogouad")-1]
        self.list_file = self.conf["ad_log_list_dir"] + "list_" + self.tm_str + "_"+ suffix
        self.log_file = log_file
        self.done_file = self.log_file + ".done"
        if os.path.isfile(self.done_file):
            print "find %s, exit" % self.done_file
            sys.exit()
        if os.path.isfile(self.list_file):
            os.remove(self.list_file)

    def do_extract(self):
        log_fd = open(self.log_file,"r")
        list_fd = open(self.list_file,"a")
        print "[%s] genHtml....%s" % (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),os.path.basename(self.log_file))
        #遍历读取原始日志文件
        for log_info in log_fd:
            if log_info.find(self.conf["sogou_channel_pid"]) < 0:
                continue
            json_str = log_info[log_info.find("[Sogou-Ad-Log]")+len("[Sogou-Ad-Log]"):-1]
            head_str = log_info[0:log_info.find("[INFO]")]
            html_info = dict()
            try:
                html_info = simplejson.loads(json_str,strict=False)
            except BaseException,arg:
                print arg
                print "load json failed"
                print log_info[0:log_info.find("\"html\"")]
                print log_info[-100:]
                print "------------------------------------------------------"
                continue
            #处理一下访问时间
            time_ret = re.search(r"(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2})",head_str)
            visit_time = "NoneVistiTime"
            if time_ret:
                visit_time = time_ret.group()
            #基本的校验之
            if len(html_info) < 3:
                print "invalid html_info length"
                continue
            if not html_info.has_key('query'):
                print "No key 'query' in html_info"
                continue
            if not html_info.has_key("html"):
                print "No key 'html' in html_info"
                continue
            if not html_info['query']:
                continue
            query = unquote(html_info['query'])
            #把query中的\t替换成空格
            if query.find("\t"):
                query = query.replace("\t"," ")
            if len(query) <= 0:
                print "query is empty"
                continue
            #远程IP地址
            remoteAddr = html_info['remoteAddr']
            #获取区域信息，若找不到则用IP来获取
            area = html_info['area']
            if area:
                area = area.replace("\t","-")
            elif remoteAddr:
                ip_str = IP.find(remoteAddr)
                ip_str = ip_str.split("\t") if ip_str else ["","none"]
                area = "-".join(ip_str)
            else:
                area = "none"
            #html的源码及uuid
            html = html_info['html'].strip()
            uuid = html_info['uuid']
            if len(html) < 10000:
                continue
            if len(uuid) <= 0:
                uuid = "none"
            if len(remoteAddr) <= 0:
                remoteAddr = "127.0.0.1"

            #处理一下html文件
            html = html.replace("\\\"","\"")
            html = html.replace("\\\\","\\")
            html = html.replace("<head>","<head><base href=\"http://www.sogou.com/\">")
            html_dir = self.conf['html_file_dir'] % self.tm_str
            #存放html文件的目录，不存在则创建之
            dir = html_dir.rstrip("/")
            if not os.path.isdir(dir):
                try:
                    os.makedirs(dir)
                except Exception,args:
                    print args
            #拼装html文件的路径，并写入
            html_file = dir + "/" + hashlib.md5(query.encode("utf-8")).hexdigest()
            html_file += "-" +uuid + ".html"
            if not os.path.isfile(html_file):
                fp = open(html_file,"w+")
                fp.write(html.encode())
                fp.close()

            #list目录信息：query、province-city、time、remoteIP、html_file
            lt = query + "\t" + area + "\t" + visit_time + "\t" + remoteAddr + "\t" + html_file + "\n"
            list_fd.write(lt)
            list_fd.flush()
        #清理资源
        log_fd.close()
        list_fd.close()
        os.system("touch %s" % self.done_file)
        return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "USAGE %s %s" % (sys.argv[0],"utf8_log_file")
        sys.exit()
    log_file = sys.argv[1]
    if not os.path.isfile(log_file):
        print "read log file failed %s" % log_file
        sys.exit()
    obj = com_extract(log_file)
    obj.do_extract()