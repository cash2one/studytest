# -*- coding: utf-8 -*-


from common.DB import DB
from common.utils import utils
from common.config import *
from urllib import unquote
import sys, re, redis,simplejson

reload(sys)
sys.setdefaultencoding('utf-8')


db_master = DB(**mysql_host["db_master"])
redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)

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
