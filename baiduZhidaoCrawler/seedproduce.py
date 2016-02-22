# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import *
from urllib import unquote
import os, sys, redis

reload(sys)
sys.setdefaultencoding('utf-8')

db = DB(**mysql_host["db_slave"])
psize = 1000000;

# 先将所有的问题ID灌入到Redis里去
redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)
startID = 0
while True:
    qidlist = db.fetch_all(
            "SELECT `qid`,`id` FROM `question` WHERE `id` >" + str(startID) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(qidlist) <= 0:
        break
    for id in qidlist:
        qid = str(id["qid"])
        startID = str(id["id"])
        print startID
        r.sadd(baidu_zhidao_qid, qid)

# 往种子搜索词队列里写入
sid = r.get(max_seed_id)
startId = int(sid) if sid else 0
while True:
    wordlist = db.fetch_all(
            "SELECT * FROM `seedword` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        startId = word["id"]
        print startId
        r.set(max_seed_id, startId)
        w = word["word"].strip()
        w = unquote(w)
        r.rpush(seedword_redis_queue, w)