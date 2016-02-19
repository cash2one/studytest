# -*- coding: utf-8 -*-

# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: http://doc.scrapy.org/en/latest/topics/item-pipeline.html

from common.DB import DB
from common.config import *
from urllib import unquote
import redis


class qaPipeline(object):
    slave = DB(**mysql_host["db_slave"])
    master = DB(**mysql_host["db_master"])
    redis_pool = redis.ConnectionPool(**redis_host["master"])

    def process_item(self, item, spider):
        qId = item.get("qId", 0)
        qTitle = item.get("qTitle", "")
        qContent = item.get("qContent", "")
        qTag = item.get("qTag", "")
        qUrl = item.get("qUrl", "")
        aId = item.get("aId", 0)
        aContent = item.get("aContent", "")
        aType = item.get("aType", 0)

        # 连接数据库，写入数据
        if qId and qTitle and aId and aContent:
            sql = "SELECT COUNT(*) AS `total` FROM `question` WHERE `qid`='" + qId + "'";
            ret = self.slave.fetch_row(sql)
            ret = ret["total"]
            if not ret:
                iData = dict({
                    "qid": qId,
                    "title": unquote(qTitle),
                    "content": unquote(qContent),
                    "tag": unquote(qTag),
                    "url": unquote(qUrl)
                })
                uData = dict({"title": unquote(qTitle), "content": unquote(qContent), "tag": unquote(qTag),})
                self.master.insert_update("question", iData, uData)

                # 写入回答信息
                insertData = dict({"aid": aId, "qid": qId, "content": unquote(aContent), "type": aType})
                updateData = dict({"content": unquote(aContent), "type": aType})
                self.master.insert_update("answer", insertData, updateData)

                # 问题的标题作为种子词继续搜索
                self.master.insert("seedword", word=unquote(qTitle))

                # 问题的ID添加到Redis里
                r = redis.Redis(connection_pool=self.redis_pool)
                r.sadd(baidu_zhidao_qid, qId)

        return item