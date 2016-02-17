#!/usr/local/bin/python
# coding=gbk

################################################################
# 各个引擎都通用的工具类，全是静态方法
################################################################

import re,sys,string,os
from dbase import mark_oss
from bs4 import BeautifulSoup,Comment,Tag,NavigableString

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
    # 解析配置文件，返回一个dict
    ##########################################################################################
    @staticmethod
    def parseConf(config_file):
        fp = open(config_file,"r")
        conf = dict()
        for item in fp:
            item = item.strip(string.whitespace)
            #校验基本的合法性
            if len(item) <= 0 or item.startswith("#") or item.find("=") < 0:
                continue
            tmp = item.split("=")
            key = tmp[0].strip(string.whitespace)
            value = tmp[1].strip(string.whitespace)
            value = value.strip('"')
            if len(key) <= 0 or len(value) <= 0:
                continue
            #校验是否以[]结尾
            if key.endswith("[]"):
                key = key.rstrip("[]")
                key = key.strip(string.whitespace)
                if conf.has_key(key):
                    conf[key].append(value)
                else:
                    conf[key] = list([value])
                continue
            
            conf[key] = value
        return conf

    ##########################################################################################
    #解析字符串，将形如a=2&b=4解析成字典形式的
    ##########################################################################################
    @staticmethod
    def parseStr(str):
        arg = str.split('&')
        ret = dict()
        for param in arg:
            tmp = param.split('=')
            if len(tmp) < 2 or not tmp[1]:
                continue
            ret[tmp[0]] = tmp[1]

        return ret

    ##########################################################################################
    #确保指定文件所在的目录一定存在，否则就创建目录，失败就返回False
    ##########################################################################################
    @staticmethod
    def mk_sure_dir(f):
        if os.path.isfile(f):
            return True
        dn = os.path.dirname(f)
        if not os.path.isdir(dn):
            return os.makedirs(dn,0755)
        return True

    ##########################################################################################
    #检测百度的搜索结果页是否被封掉了，即要输入验证码
    ##########################################################################################
    @staticmethod
    def is_baidu_verifycode(html_string):
        #过滤掉注释
        doc_soup = BeautifulSoup(html_string,"lxml")
        comments = doc_soup.findAll(text=lambda text:isinstance(text, Comment))
        for comment in comments:
            comment.extract()
        nodes = doc_soup.select("div > form")
        if len(nodes) <= 0:
            return False
        form = nodes[0]
        if "action" in form.attrs and form["action"].find("verify.baidu.com/verify")>=0:
            return True
        return False