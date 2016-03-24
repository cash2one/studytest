# -*- coding: utf-8 -*-


from common.DB import DB
from common.utils import utils
from common.config import *
from urllib import unquote
import sys, re, redis

reload(sys)
sys.setdefaultencoding('utf-8')


redis_pool = redis.ConnectionPool(**redis_host["master"])
r = redis.Redis(connection_pool=redis_pool)


db_slave = DB(**mysql_host["db_slave"])
db_master = DB(**mysql_host["db_master"])
psize = 1000000;

sid = r.get(wash_max_seed_id)
startId = int(sid) if sid else 0
while True:
    wordlist = db_slave.fetch_all("SELECT `title`,`id`,`content` FROM `question` WHERE `id` > " + str(startId) + " ORDER BY `id` ASC LIMIT " + str(psize))
    if len(wordlist) <= 0:
        break
    for word in wordlist:
        startId = word["id"]
        print startId
        title = word["title"].strip()
        segTitle = utils.split_str(title)
        for seedtitle in segTitle:
            db_master.insert("seedword", word=seedtitle.strip())
        if len(title) < 15 and not title.isalnum():
            db_master.insert("seedword", word=title.strip())
        content = word["content"].strip()
        segContent = utils.split_str(content)
        for seedcontent in segContent:
            db_master.insert("seedword", word=seedcontent.strip())
        r.set(wash_max_seed_id, startId)