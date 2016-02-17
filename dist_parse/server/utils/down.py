#!/usr/local/bin/python
# coding=utf-8


################################################################
# 下载工具类
################################################################
import StringIO
import pycurl,sys,os,random
from conf import *
from utils import *
reload(sys)
sys.setdefaultencoding('utf-8')

class down:
    def __init__(self, proxy):
        self.curl_handle = pycurl.Curl()
        self.buffer = None
        self.proxy_host = proxy
        self.__set_opt__()

    ##########################################################################################
    # 添加了重试机制，如果不用代理时失败则重新用代理抓取一次
    ##########################################################################################
    def getHtml(self,url):
        is_use_proxy = 0
        local_ip = utils.get_local_ip()
        if use_proxy.has_key(local_ip):
            is_use_proxy = use_proxy[local_ip]
        try:
            return self.__get_content__(url,is_use_proxy)
        except Exception,args:
            return self.__get_content__(url,1)

    ##########################################################################################
    # 获取页面的内容
    ##########################################################################################
    def __get_content__(self, url, is_use_proxy):
        self.url = url
        self.buffer = StringIO.StringIO()

        #设置相关
        if len(self.proxy_host) > 0 and self.proxy_host in proxy_list and int(is_use_proxy) > 0:
            self.curl_handle.setopt(pycurl.PROXY, self.proxy_host)

        self.curl_handle.setopt(pycurl.WRITEFUNCTION, self.buffer.write)
        self.curl_handle.setopt(pycurl.URL, self.url)
        self.curl_handle.perform()

        #错误状态码
        ret_status = self.curl_handle.getinfo(self.curl_handle.HTTP_CODE)
        if int(ret_status) != 200:
            raise Exception("something wrong occured errmsg[%s] http_code[%d]" % (
                self.curl_handle.errstr(),int(ret_status)
            ))
        html = self.buffer.getvalue()
        self.buffer.close()

        #是否被下载残疾了
        if html.find("</html>") <= 0 or html.find("</body>") <= 0:
            raise Exception("incomplete html content")

        #大小检查
        if len(html) <= 0:
            raise("empty html content")

        #检测是否被封掉了
        if len(html) < 102400 and url.find("baidu.com") > 0:
            utils.check_baidu_forbid(html)

        return html

    ##########################################################################################
    # 对curl进行设置
    ##########################################################################################
    def __set_opt__(self):
        self.curl_handle.setopt(pycurl.FOLLOWLOCATION, 2)
        self.curl_handle.setopt(pycurl.MAXREDIRS, 3)
        self.curl_handle.setopt(pycurl.CONNECTTIMEOUT, 60)
        self.curl_handle.setopt(pycurl.NOSIGNAL, 1)
        #self.curl_handle.setopt(pycurl.PROXY,"http://11.11.11.11:8080")
        self.curl_handle.setopt(pycurl.HTTPHEADER,[
            "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3",
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
            "Accept-Charset: utf-8;q=0.7,*;q=0.7",
            "Connection: keep-alive",
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0"
        ])
        return True