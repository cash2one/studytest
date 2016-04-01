# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import *
from urllib import unquote
import os, sys, redis

reload(sys)
sys.setdefaultencoding('utf-8')

redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)

db = DB(**mysql_host["db_slave"])
db_master = DB(**mysql_host["db_master"])
sid = r.get(move_seed_id)
startId = int(sid) if sid else 0
fp = open("seedword.log","a")
cnt = 0
tmpList = []
while True:
    wordlist = db.fetch_all("SELECT * FROM `seedword` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT 100000")
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        cnt = cnt + 1
        startId = word["id"]
        w = word["word"].strip()
        w = unquote(w)
        sql = "INSERT IGNORE INTO `seedword` SET `word`='" + db_master.conn.escape_string(w) + "';\n"
        tmpList.append(sql)
        if cnt % 1000 == 0 and len(tmpList) > 0:
            fp.writelines(tmpList)
            r.set(move_seed_id, startId)
            tmpList = []
            print cnt
if len(tmpList) > 0:
    fp.writelines(tmpList)
    r.set(max_seed_id, startId)
    tmpList = []
    print cnt
fp.close()