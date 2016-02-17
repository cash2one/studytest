#!/usr/local/bin/python
# -*- coding: utf-8 -*-



from utils.DB import *
from webpage_parser import WebPageParser
from webpage_config import WebPageConfig
from conf import db_conf
import re
import os
import sys
import logging,urllib
from webpage_parser_normal import WebPageParser_normal
from urlparse import urlparse


log_name = "parser.log"
logger = logging.getLogger(os.path.basename(__file__))
logger.setLevel(logging.DEBUG)
formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
file_handler = logging.FileHandler(log_name, "w")
file_handler.setFormatter(formatter)
logger.addHandler(file_handler)

conf_file = "./conf/srpp-config-sogou-no-ad.xml"
config = WebPageConfig(logger)
if config.ParseFile(conf_file) != 0:
    logger.error("Parse config file failed! file="+conf_file)
    sys.exit()
parser = WebPageParser(logger)
if parser.Init(config) != 0:
    logger.error("Init WebPageParser failed!")
    sys.exit()

conf_file_wenwen = "./conf/srpp-config-sogou-wenwen.xml"
config_wenwen = WebPageConfig(logger)
if config_wenwen.ParseFile(conf_file_wenwen) != 0:
    logger.error("Parse config file failed! file="+conf_file_wenwen)
    sys.exit()
parser_wenwen = WebPageParser_normal(logger)
if parser_wenwen.Init(config_wenwen) != 0:
    logger.error("Init WebPageParser failed!")
    sys.exit()



snapshot_pattern1 = "http://10.11.195.131/getsnapshot?tid=2301&query=%s&page_type=page_search&engine_id=1"
snapshot_pattern46 = "http://10.11.195.131/getsnapshot?tid=2301&query=%s&page_type=page_search&engine_id=46"


fname = "./wenwen.csv"
if os.path.isfile(fname):
    os.remove(fname)
fp = open(fname,"w")

db = DB(**db_conf)
sql = "SELECT query FROM `tb_dcg_data_list` WHERE task_id =2301 AND ENGINE =1 AND title LIKE '%搜狗问问%' GROUP BY query"
ret_list = db.fetch_all(sql)
for item in ret_list:
    query = item["query"].encode("utf8")
    print query
    url = snapshot_pattern1 % query
    parser.Reset()
    parser_wenwen.Reset()
    #正常结果页解析之
    parser.ParseHTML(urllib.urlopen(url).read().encode("utf8"))
    wenwen_len = len(parser.wenwen)
    if  wenwen_len <= 0:
        continue
    rank = 1
    for tmp in parser.wenwen:
        ostr = "\"%s\",\"%s\",%s,sogou,\"%s\"\n" % (query,tmp["url"].encode("utf8"),str(rank),url)
        fp.write(ostr)
        rank += 1
    #wenwen结果页解析之
    rank = 1
    url_wenwen = snapshot_pattern46 % query
    parser_wenwen.ParseHTML(urllib.urlopen(url_wenwen).read().encode("utf8"))
    for index,result in enumerate(parser_wenwen.page_element_list):
        if result.search_result == None or rank > wenwen_len:
            continue
        url = result.search_result.url if len(result.search_result.url) > 0 else result.search_result.raw_url
        tmp_url = url.encode("utf8")
        tmp = urlparse(tmp_url)
        if tmp.netloc == "wenwen.sogou.com":
            url = "%s://%s%s" % (tmp.scheme,tmp.netloc,tmp.path)
        ostr = "\"%s\",\"%s\",%s,wenwen,\"%s\"\n" % (query,url,str(rank),url_wenwen)
        fp.write(ostr)
        rank += 1


fp.close()

