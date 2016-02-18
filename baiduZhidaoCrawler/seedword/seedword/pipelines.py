# -*- coding: utf-8 -*-

# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: http://doc.scrapy.org/en/latest/topics/item-pipeline.html
from common.DB import DB
from common.config import mysql_host
from urllib import unquote
import time


class seedwordPipeline(object):
    def process_item(self, item, spider):
        seedword = item.get("seedword", "")
        if seedword:
            db = DB(**mysql_host["db_lmcrawl_master"])
            db.insert("seedword", word=seedword)
        return item
