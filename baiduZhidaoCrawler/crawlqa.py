# -*- coding: utf-8 -*-

from common.DB import DB
from common.config import *
from urllib import unquote
import os, sys, redis

reload(sys)
sys.setdefaultencoding('utf-8')

# 从Redis队列里读
redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)
while True:
    w = r.blpop(seedword_redis_queue, 5)
    if not w:
        continue
    w = w[1]
    if len(w) <= 0:
        continue
    cmd = "cd ./qa;scrapy crawl qa -a query=\"%s\"" % w.strip()
    os.system(cmd)