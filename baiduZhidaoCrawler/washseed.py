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


tmpList = []
cnt = 0
while True:
    title = r.blpop(wash_seed_queue, 5)
    if not title:
        continue
    title = title[1]
    if len(title) <= 0:
        continue
    segTitle = utils.split_str(title)
    cnt = cnt + 1
    for seedtitle in segTitle:
        tmpList.append(seedtitle)
        tmpList.append(0)
    if cnt % 100000 == 0:
        r.zadd(tmp_seed_list,*tmpList)
        tmpList = []
        print cnt
if tmpList:
    r.zadd(tmp_seed_list,*tmpList)
