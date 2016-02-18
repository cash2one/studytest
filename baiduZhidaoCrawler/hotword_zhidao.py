# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import mysql_host
from urllib import unquote
import os, sys

reload(sys)
sys.setdefaultencoding('utf-8')

db = DB(**mysql_host["db_lmcrawl_slave"])
list = db.fetch_all("select `word` from seedword")
for word in list:
    w = word["word"].strip()
    w = unquote(w)
    cmd = "cd ./qa;scrapy crawl qa -a query=\"%s\"" % w
    os.system(cmd)

list = db.fetch_all("select cname from category")
for word in list:
    w = word["cname"].strip()
    w = unquote(w)
    cmd = "cd ./qa;scrapy crawl qa -a query=\"%s\"" % w
    os.system(cmd)

list = db.fetch_all("select tag from tag")
for word in list:
    w = word["tag"].strip()
    w = unquote(w)
    cmd = "cd ./qa;scrapy crawl qa -a query=\"%s\"" % w
    os.system(cmd)
