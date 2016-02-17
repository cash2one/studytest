#!/usr/local/bin/python
# -*- coding: utf-8 -*-

################################################################
# 各个引擎都通用的工具类，全是静态方法
################################################################

import re,sys,string,os
reload(sys)
sys.setdefaultencoding('utf-8')

class Utils:

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
    # 解析配置文件，读到字典里
    ##########################################################################################
    @staticmethod
    def parseConfig(config_file):
        ret = dict()
        fp = open(config_file,"r")
        for line in fp:
            tmp = line.strip()
            if len(tmp) <= 0 or tmp.startswith("#"):
                continue
            if tmp.find("=") <= 0:
                continue
            pos = tmp.find("=")
            key = tmp[0:pos].strip()
            value = tmp[pos+1:].strip()
            if key.endswith("[]"):
                tmp_key = key.rstrip("[]")
                if tmp_key not in ret:
                    ret[tmp_key] = list()
                ret[tmp_key].append(value)
            else:
                ret[key]=value
        fp.close()
        return ret