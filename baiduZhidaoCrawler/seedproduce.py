# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import *
from urllib import unquote
import os, sys, redis
from common.utils import utils

reload(sys)
sys.setdefaultencoding('utf-8')

db = DB(**mysql_host["db_slave"])
db_master = DB(**mysql_host["db_master"])
psize = 10000000
chunk = 10000

redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)


# 往种子搜索词队列里写入
qlen = int(r.llen(seedword_redis_queue))
if qlen < 100000:
    cnt = 0
    sid = r.get(max_seed_id)
    startId = int(sid) if sid else 0
    tmpList = []
    wordlist = db.fetch_all("SELECT * FROM `seedword` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT 100000")
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
cnt = 0
initSql = "INSERT IGNORE INTO `seedword` (`word`) VALUES ('"
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
        seglist = utils.split_str(word["title"])
        for seg in seglist:
            tmpList.append( db_master.conn.escape_string(unquote(seg.strip())))
        if cnt % chunk == 0:
            sql = initSql + "'),('".join(tmpList) + "')"
            db_master.execute(sql)
            tmpList = []
            r.set(wash_max_seed_id, startId)
            print cnt
if tmpList:
    r.set(wash_max_seed_id, startId)
    sql = initSql + "'),('".join(tmpList) + "')"
    db_master.execute(sql)