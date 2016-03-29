# -*- coding: utf-8 -*-

# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: http://doc.scrapy.org/en/latest/topics/item-pipeline.html
from common.DB import DB
from common.config import *
from urllib import unquote
import redis

class topwordPipeline(object):

    redis_pool = redis.ConnectionPool(**redis_host["master"])
    r = redis.Redis(connection_pool=redis_pool)

    def process_item(self, item, spider):
        topword = item.get("topword", "")
        if topword:
            db = DB(**mysql_host["db_master"])
            topword = unquote(topword)
            print topword
            db.insert("topword", word=topword)
        return item
