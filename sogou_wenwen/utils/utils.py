#!/usr/local/bin/python
# coding=utf-8


################################################################
# 各个引擎都通用的工具类，全是静态方法
################################################################

import re,sys,os,random
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