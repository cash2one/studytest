# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import mysql_host
from common.config import redis_host
from urllib import unquote
import os, sys, redis

reload(sys)
sys.setdefaultencoding('utf-8')

db = DB(**mysql_host["db_slave"])
psize = 100000;

# 先将所有的问题ID灌入到Redis里去
redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)
startQid = 0
while True:
    qidlist = db.fetch_all(
            "SELECT `qid` FROM `question` WHERE `qid` >" + str(startQid) + " ORDER BY `qid` ASC LIMIT " + str(psize))
    if len(qidlist) <= 0:
        break
    for id in qidlist:
        qid = str(id["qid"])
        startQid = qid
        r.sadd("baidu_zhidao_qid", qid)

# 开始查询所有的种子关键词进行抓取
max_seed_id = "baidu_zhidao_maxseed_id"
sid = r.get(max_seed_id)
startId = int(sid) if sid else 0
while True:
    wordlist = db.fetch_all(
        "SELECT * FROM `seedword` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        startId = word["id"]
        r.set(max_seed_id,startId)
        w = word["word"].strip()
        w = unquote(w)
        cmd = "cd ./qa;scrapy crawl qa -a query=\"%s\"" % w
        os.system(cmd)