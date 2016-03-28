# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import *
from urllib import unquote
import os, sys, redis

reload(sys)
sys.setdefaultencoding('utf-8')

db = DB(**mysql_host["db_slave"])
psize = 10000000
chunk = 500000

redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)


#所有的种子添加到sorted set里
cnt=0
sid = r.get(tmp_seed_list_id)
startId = int(sid) if sid else 0
tmpList = []
while True:
    wordlist = db.fetch_all("SELECT * FROM `seedword` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        cnt = cnt + 1
        startId = word["id"]
        w = word["word"].strip()
        w = unquote(w)
        score = int(startId)
        tmpList.append(w)
        tmpList.append(score)
        if cnt % chunk == 0:
            r.zadd(tmp_seed_list,*tmpList)
            r.set(tmp_seed_list_id, startId)
            tmpList = []
            print startId
if tmpList:
    r.zadd(tmp_seed_list,*tmpList)
    r.set(tmp_seed_list_id, startId)
    print startId


# 往种子搜索词队列里写入
cnt=0
sid = r.get(max_seed_id)
startId = int(sid) if sid else 0
tmpList = []
while True:
    wordlist = db.fetch_all("SELECT * FROM `seedword` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        cnt = cnt + 1
        startId = word["id"]
        w = word["word"].strip()
        w = unquote(w)
        tmpList.append(w)
        if cnt % chunk == 0:
            r.rpush(seedword_redis_queue, *tmpList)
            r.set(max_seed_id, startId)
            tmpList = []
            print startId
if tmpList:
    r.rpush(seedword_redis_queue, *tmpList)
    r.set(max_seed_id, startId)
    print startId


#往队列里塞东西
cnt=0
sid = r.get(wash_max_seed_id)
startId = int(sid) if sid else 0
tmpList = []
while True:
    wordlist = db.fetch_all("SELECT `title`,`id` FROM `question` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        cnt = cnt + 1
        startId = word["id"]
        tmpList.append( word["title"].strip())
        if cnt % chunk == 0:
            r.set(wash_max_seed_id, startId)
            r.rpush(wash_seed_queue,*tmpList)
            tmpList = []
            print startId
if tmpList:
    r.set(wash_max_seed_id, startId)
    r.rpush(wash_seed_queue,*tmpList)
    print startId
