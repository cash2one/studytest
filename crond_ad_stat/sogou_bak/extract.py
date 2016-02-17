#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 解析线程：从队列里获取日志文件，然后生成html文件、解析html文件
# 将抽取的搜索结果条写到result文件里
################################################################

from utils.utils import *
import threading,simplejson
from urllib import unquote
import time,datetime,os,sys,random,re
import hashlib
import IP
reload(sys)
sys.setdefaultencoding('utf-8')

class sogou_extract(threading.Thread):
    def __init__(self, config):
        threading.Thread.__init__(self)
        self.conf=config
        self.tm_str = time.strftime('%Y%m%d',time.localtime(utils.getThriDate()))
        done_log_file = self.conf["done_log_file"] % self.tm_str
        self.done_fd = open(done_log_file,"a+")
        self.done_set = set()
        for f in self.done_fd:
            self.done_set.add(f.strip())

    def run(self):
        while True:
            try:
                #转为utf8之后的原始日志文件，绝对路径
                fileName = self.conf["extractQueueObj"].get(True,5)
            except Exception,args:
                print "get extractQueueObj error",args
                time.sleep(random.randint(1,5))
                continue
            if not fileName:
                continue
            bname = os.path.basename(fileName)
            print "[%s] getQueue %s" % (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),bname)
            suffix = bname[0:bname.find("sogouad")-1]
            list_file = self.conf["ad_log_list_dir"] + "list_" + self.tm_str + "_"+ suffix
            if os.path.isfile(list_file):
                self.conf["parseQueueObj"].put(list_file.strip(),True)
            else:
                #抽取html文件
                self.__gen_html( fileName, list_file)
            self.done_fd.write(fileName+"\n")
            self.done_fd.flush()
        return True

    ###########################################################################
    #生成html页面，并写到list文件里
    ###########################################################################
    def __gen_html(self, log_file, list_file):
        log_fd = open(log_file,"r")
        list_fd = open(list_file,"a+")
        print "[%s] genHtml....%s" % (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),os.path.basename(log_file))
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

        #将list文件放进解析队列，有可能会出错抛出异常
        for i in range(3):
            try:
                self.conf["parseQueueObj"].put(list_fd.name,True)
                self.done_set.add(log_file)
            except Exception,args:
                print args
                time.sleep(random.randint(1,5))
        print "[%s] addQueue %s" % (time.strftime('%Y-%m-%d %H:%I:%S',time.localtime(time.time())),os.path.basename(log_file))

        return True