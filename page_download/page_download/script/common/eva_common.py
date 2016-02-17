#! /usr/local/bin/python
# -*- coding: gbk -*-
#######################################################
# Copyright (c) 2012, Tencent Inc.
# All rights reserved
#
# Author: Manzhao BU <chillbu@tencent.com>
#
# [TODO]description
#######################################################

import os
import sys
import MySQLdb
import logging
import random
import urlparse
import urllib2,urllib,httplib
import hashlib

kMaxResultRank = 5

kDBInfo = {"new_pi":{"ip":"10.151.138.141", \
              "user":"root", \
              "passwd":"root141", \
              "db":"NewPIEvaSys", \
              "desc":"New PI db"}, \
           "bigdipper":{"ip":"10.151.138.141", \
               "user":"root", \
               "passwd":"root141", \
               "db":"NewSatisfactionSys", \
               "desc":"bigdipper db"}, \
           "crowd":{"ip":"10.150.163.36", \
              "user":"root", \
              "passwd":"root36", \
              "db":"crowd", \
              "desc":"crowd online db"}, \
           "crowd_test":{"ip":"10.11.215.30", \
               "user":"root", \
               "passwd":"root224", \
               "db":"crowd", \
               "desc":"crowd test db"}, \
            "uni_eva":{"ip":"10.11.215.30", \
               "user":"root", \
               "passwd":"root36", \
               "db":"uni_eva", \
               "desc":"union eva system"}, \
            "uni_eva_test":{"ip":"10.11.215.30", \
               "user":"root", \
               "passwd":"root30", \
               "db":"uni_eva", \
               "desc":"union eva system test"}, \
           "srss":{"ip":"10.11.215.30", \
               "user":"root", \
               "passwd":"root30", \
               "db":"srss", \
               "desc":"search result structure system"}, \
     	   "srss_for_eva":{"ip":"10.11.215.30", \
               "user":"root", \
               "passwd":"root30", \
               "db":"srss_for_eva", \
               "desc":"search result structure system for eva"}}


def PrintDBInfo():
    for item in kDBInfo.keys():
        print "#######################"
        print "name: %s" % (item)
        print "desc: %s" % (kDBInfo[item]["desc"])
        print "ip: %s" % (kDBInfo[item]["ip"])
        print "user: %s" % (kDBInfo[item]["user"])
        print "db: %s" % (kDBInfo[item]["db"])               
               
#######################################################
#
# Utility class
#
#######################################################
class DBConnection:
    "For handle DB connection"
    def __init__(self):
        self.db_server_ip = ""
        self.db_server_user = ""
        self.db_server_passwd = ""
        self.db_name = ""
        self.connection = None

    def Connect(self, ip, user, passwd, name):
        self.db_server_ip = ip
        self.db_server_user = user
        self.db_server_passwd = passwd
        self.db_name = name
        try:
            self.connection = MySQLdb.connect(host = self.db_server_ip, user = self.db_server_user, passwd = self.db_server_passwd, db = self.db_name)
            cursor = self.connection.cursor()
            cursor.execute("SET NAMES gbk")
            cursor.close()
        except Exception, e:
            print "Error %d: %s" % (e.args[0], e.args[1])
            sys.exit(1)

    def ConnectByName(self, name):
        self.Connect(kDBInfo[name]["ip"], \
                     kDBInfo[name]["user"], \
                     kDBInfo[name]["passwd"], \
                     kDBInfo[name]["db"]);
    
    def Close(self):
        self.connection.close()

#######################################################
#
# DB utility functions
#
#######################################################
def GetParameter(param_list):
    param = {}
    for p in param_list[1:]:
        kv = p.split("=")
        param[kv[0]] = kv[1]
        print "%s=%s" % (kv[0], kv[1])
    return param

def SelectMaxRound(db):
    try:
        cursor = db.connection.cursor()
        cursor.execute("SELECT MAX(round) FROM round")
        result = cursor.fetchall()
        cursor.close()
    except Exception, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)
    return result[0][0]

def SelectMaxTaskID(db):
    try:
        cursor = db.connection.cursor()
        cursor.execute("SELECT MAX(task_id) FROM task_info")
        result = cursor.fetchall()
        cursor.close()
    except Exception, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)
    return result[0][0]

def SelectEngines(db, round):
    result = []
    try:
        cursor = db.connection.cursor()
        cursor.execute("SELECT DISTINCT(Engine) FROM resultset WHERE round=%s" % round)
        rows = cursor.fetchall()
        cursor.close()
    except Exception, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)
    for row in rows:
        result.append(row[0])
    return result

def SelectQueryTypes(db, round):
    result = []
    try:
        cursor = db.connection.cursor()
        cursor.execute("SELECT DISTINCT(Type) FROM query WHERE round=%s" % round)
        rows = cursor.fetchall()
        cursor.close()
    except Exception, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)
    for row in rows:
        
        result.append(row[0])
    return result
    
def GetMysqlLock(db_conn, lock_name, timeout=10):
    sql = "SELECT GET_LOCK('%s',%d)" % (lock_name, timeout)
    try:
        cursor = db_conn.connection.cursor()
        cursor.execute(sql)
        rows = cursor.fetchall()
        cursor.close()
    except Exception, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)
    return rows[0][0]
    
def ReleaseMysqlLock(db_conn, lock_name):
    sql = "SELECT RELEASE_LOCK('%s')" % (lock_name)
    try:
        cursor = db_conn.connection.cursor()
        cursor.execute(sql)
        rows = cursor.fetchall()
        cursor.close()
    except Exception, e:
        print "Error %d: %s" % (e.args[0], e.args[1])
        sys.exit(1)
    return rows[0][0]

#######################################################
#
# Srss system interface
#
#######################################################    
def SearchFeature(engine, content):
    ############ !!!NEED MODIFY IF CHANGE DIR!!! ######################
    output = os.popen("/data1/webpage/tools/srss_client --server_ip=10.11.215.30 --server_port=30025 --cmd=SearchFeature --engine="+engine+" --content=\""+content.encode('gbk','ignore')+"\"")
    out = output.read().splitlines()
    if len(out) == 0 or out[0] == "Not found" or not out[0].isdigit():
        return 0
    return out[0]
    
def SubmitFeature(content, engine, taskid, type):
    ############ !!!NEED MODIFY IF CHANGE DIR!!! ######################
    output = os.popen("/data1/webpage/tools/srss_client --server_ip=10.11.215.30 --server_port=30025 --cmd=SubmitFeature --type="+type+" --engine="+engine+" --taskid="+str(taskid)+" --content=\""+content.encode('gbk','ignore')+"\"")
    out = output.read().splitlines()
    if len(out) == 0 or out[0] == "" or not out[0].isdigit():
        return 0
    return out[0]
    
#######################################################
#
# Utility functions
#
#######################################################
'''计算DocID(新架构DocID)
    调用calc_docid程序来获得指定url的docid
'''
def GetDocID(url):
    ############ !!!NEED MODIFY IF CHANGE DIR!!! ######################
    if url.find('\'') !=0 :
        output = os.popen("./tools/calc_docid  \""+url.encode('gbk','ignore')+"\"")
    else:
        output = os.popen("./tools/calc_docid  \'"+url.encode('gbk','ignore')+"\'")

    out = output.read().splitlines()    
    if not out[0].isdigit():
        print "GetDocID error: "+url
        print "Return: "+out[0]
        return 0     
    return out[0]
    
'''
    计算指定文本的md5,取其前8位作为uint64数字
'''
def GetStrMd5(text):
    Module2_64=18446744073709551616
    md5 = hashlib.md5()
    md5.update(text)
    did = int(md5.hexdigest(), 16) % Module2_64
    return did

'''设置日志输出格式
'''
def SetLogger(name, level = logging.DEBUG):
    self.logger = logging.getLogger(name)
    self.logger.setLevel(level)
    ch = logging.StreamHandler()
    ch.setLevel(level)
    formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
    ch.setFormatter(formatter)
    self.logger.addHandler(ch)

'''有限转义，仅用于GetRedirectURL
    做有限url解码,只针对以下关键字:
        :   /   @   ?   =   &   [space] %
'''
def my_unquote(url):
    unquote_url = url
    flags = {"%3A":":","%2F":"/","%40":"@","%3F":"?","%3D":"=","%26":"&","%20":" ","%25":"%"}
    for key in flags.keys():
        unquote_url = unquote_url.replace(key,flags[key])
    return unquote_url    
    
'''取重定向URL
    获得最终跳转的URL
'''
def GetRedirectURL(url_str):
    url = urlparse.urlparse(url_str.strip())
    location=""
    page_path="%s?%s" % (url[2],url[4])
    h1=httplib.HTTPConnection(url[1], timeout=20) #设置超时时间为20秒
    try:
        h1.request("GET",page_path)
        r1 = h1.getresponse()
        if r1 is None:
            h1.close()
            return ""
        location=r1.getheader("Location")
        if location is None:
            h1.close()
            return ""
        location = my_unquote(urllib.quote(location))
    except Exception, e:
        print e
    return location

def NormalizeUrl(url, suffix = None):
#def url_fix(s, charset='utf-8'):
    '''Sometimes you get an URL by a user that just isn't a real
    URL because it contains unsafe characters like ' ' and so on.  This
    function can fix some of the problems in a similar way browsers
    handle data entered by the user:

    >>> url_fix(u'http://de.wikipedia.org/wiki/Elf (Begriffskl?rung)')
    'http://de.wikipedia.org/wiki/Elf%20%28Begriffskl%C3%A4rung%29'

    :param charset: The target charset for the URL if the url was
                    given as unicode string.
    '''
    #if isinstance(s, unicode):
    #    s = s.encode(charset, 'ignore')
    if suffix != None and len(suffix) > 0:
        absolute_url = urlparse.urljoin(suffix, url)
    else:
        absolute_url = url
    (scheme, netloc, path, qs, anchor) = urlparse.urlsplit(absolute_url)
    #path = urllib.quote(path, '/%')
    #qs = urllib.quote(qs.encode('gbk'), ':&=')
    return urlparse.urlunsplit((scheme, netloc, path, qs, anchor))
    

if __name__ == "__main__":
    print SearchFeature("soso", "|div|div@h3|div@h3@a|div@div|div@div@a|div@div@a@img|div@div@div|div@div@div@div|div@div@div@div@div|div@div@div@div@div@a|div@div@div@div@p|div@div@div@div@div|div@div@div@div@div@div|div@div@div@div@div@div@span|div@div@div@div@div@div|div@div@div@div@div@div@span|div@div@div@div@div@div@span@span|div@div@div@div@div@div@span@span@span|div@div@div@div@div@div@span@span@span@em|div@div@div@div@div|div@div@div@div|div@div@div@div@div|div@div@div@div@div@a|div@div@div@div@div|div@div@div@div@div@div|div@div@div@div@div@div@span|div@div@div@div@div@div|div@div@div@div@div@div@span|div@div@div@div@div@div@span@span|div@div@div@div@div@div@span@span@span|div@div@div@div@div@div@span@span@span@em|div@div@div@div@div|div@div@div@div|div@div@div@div@div|div@div@div@div@div@a|div@div@div@div@div|div@div@div@div@div@div|div@div@div@div@div@div@span|div@div@div@div@div@div|div@div@div@div@div@div@span|div@div@div@div@div@div@span@span|div@div@div@div@div@div@span@span@span|div@div@div@div@div@div@span@span@span@em|div@div@div@div@div")
    print SearchFeature("baidu", "baidu")
    print SubmitFeature("test", "test", "test", "哈哈")
'''
    db_conn = DBConnection()
    db_conn.ConnectByName("srss")
    print GetMysqlLock(db_conn, 'locktest')
    print GetMysqlLock(db_conn, 'locktest')
    print ReleaseMysqlLock(db_conn, 'locktest')
    print GetRedirectURL(sys.argv[1])
    print "[TEST] kMaxResultRank: ", kMaxResultRank

    print "[TEST] Test PrintDBInfo:"
    PrintDBInfo()

    pi_db = DBConnection()
    pi_db.ConnectByName("new_pi")
    print "[TEST] NewPI max round: ",SelectMaxRound(pi_db)
    print "[TEST] NewPI round 55 engines: "
    print SelectEngines(pi_db, 55)
    pi_db.Close()
    cw_test_db = DBConnection()
    cw_test_db.ConnectByName("crowd_test")
    print "[TEST] CWTest max taskid: ",SelectMaxTaskID(cw_test_db)
    cw_test_db.Close()
'''     
