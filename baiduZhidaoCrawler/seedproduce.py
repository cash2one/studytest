# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import *
from urllib import unquote
import os, sys, redis

reload(sys)
sys.setdefaultencoding('utf-8')

db = DB(**mysql_host["db_slave"])
db_master = DB(**mysql_host["db_master"])
psize = 10000000
chunk = 500000

redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)
fp = open(tmpfile,"a")

"""
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
"""

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
            tmpstr = "\n".join(tmpList) + "\n"
            fp.write(tmpstr)
if tmpList:
    tmpstr = "\n".join(tmpList) + "\n"
    fp.write(tmpstr)
fp.close()


cnt=0
psize = 10000
chunk = 500
startId = 0
tmpSeedList = []
fploc = int(r.get(tmpfile_seek))
initSql = "INSERT IGNORE INTO `seedword` (`word`) VALUES ('"
fp = open(tmpfile,"a")
fp.seek(fploc)
for word in fp:
    w = word.strip()
    if len(w) <= 0:
        continue
    cnt = cnt + 1
    tmpSeedList.append(w)
    if cnt % chunk == 0:
        sql = initSql + "'),('".join(tmpSeedList) + "')"
        db_master.execute(sql)
        tmpList = []
        print cnt
        r.set(tmpfile_seek,fp.tell())
if tmpSeedList:
    sql = initSql + "'),('".join(tmpSeedList) + "')"
    db_master.execute(sql)