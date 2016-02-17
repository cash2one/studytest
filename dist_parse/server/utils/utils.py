#!/usr/local/bin/python
# coding=utf-8


################################################################
# 各个引擎都通用的工具类，全是静态方法
################################################################

import re,sys,os,random
from bs4 import BeautifulSoup,Comment,Tag
reload(sys)
sys.setdefaultencoding('utf-8')

class utils:

    ##########################################################################################
    # 对html源码做解析前的简单处理
    ##########################################################################################
    @staticmethod
    def strip_tags(html_string):
        if len(html_string) <= 0:
            return ""
        #去掉script标签
        omit_re = re.compile(u"(<script.*?>.*?<\/script>)",re.S|re.I)
        ret = omit_re.split(html_string)
        new_html_string = "";
        for tmp in ret:
            tmp = tmp.strip()
            if len(tmp) <= 0:
                continue
            if tmp.find("</script>") >= 0:
                continue
            new_html_string += tmp
        #去掉style标签
        omit_re = re.compile(u"(<style.*?>.*?<\/style>)",re.S|re.I)
        ret = omit_re.split(new_html_string)
        html_string = "";
        for tmp in ret:
            tmp = tmp.strip()
            if len(tmp) <= 0:
                continue
            if tmp.find("</style>") >= 0:
                continue
            html_string += tmp
        #去掉link标签
        omit_re = re.compile(u"(<link.*?>)",re.S|re.I)
        ret = omit_re.split(html_string)
        new_html_string = "";
        for tmp in ret:
            tmp = tmp.strip()
            if len(tmp) <= 0:
                continue
            if tmp.find("<link ") >= 0:
                continue
            new_html_string += tmp
        #去掉img标签
        omit_re = re.compile(u"(<img.*?>)",re.S|re.I)
        ret = omit_re.split(new_html_string)
        html_string = "";
        for tmp in ret:
            tmp = tmp.strip()
            if len(tmp) <= 0:
                continue
            if tmp.find("<img ") >= 0:
                continue
            html_string += tmp

        #若干简单的替换
        html_string = html_string.strip()
        html_string = html_string.replace("&amp;","&")
        html_string = html_string.replace("&quot;","\"")
        html_string = html_string.replace("&nbsp;"," ")
        html_string = html_string.replace("&lt;","<")
        html_string = html_string.replace("&gt;",">")
        html_string = html_string.replace("<em>","")
        html_string = html_string.replace("</em>","")

        return html_string

    ##########################################################################################
    # 获取指定长度的随机字符串
    ##########################################################################################
    @staticmethod
    def get_random_str(length):
        ret = ""
        length = int(length)
        if length <= 0:
            return ret
        while len(ret) < length:
            ret += random.choice('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789')
        return ret

    ##########################################################################################
    # 获取本机的IP地址
    ##########################################################################################
    @staticmethod
    def get_local_ip():
        ret = ""
        fp = os.popen("ifconfig | grep 'inet addr:' | grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}' ","r").read()
        lt = fp.strip().split("\n")
        if len(lt) > 0:
            return lt[0].strip()
        return ret

    ##########################################################################################
    # 检查下载的百度页面是否被封掉了
    ##########################################################################################
    @staticmethod
    def check_baidu_forbid(html_content):
        is_form = False
        is_img = False
        soup = BeautifulSoup(html_content,"lxml")
        nodes = soup.findAll("form")
        for node in nodes:
            if "action" in node.attrs:
                if node.attrs["action"].find("verify.baidu.com") > 0:
                    is_form = True
        nodes = soup.findAll("img")
        for node in nodes:
            if "src" in node.attrs:
                if node.attrs["src"].find("verify.baidu.com") > 0:
                    is_img = True
        if is_form and is_img:
            raise Exception("forbidden by baidu")
        return True

    ##########################################################################################
    # 检查下载的360页面是否被封掉了
    ##########################################################################################
    @staticmethod
    def check_360_forbid(html_content):
        return True