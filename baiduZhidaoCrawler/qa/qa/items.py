# -*- coding: utf-8 -*-

# Define here the models for your scraped items
#
# See documentation in:
# http://doc.scrapy.org/en/latest/topics/items.html

import scrapy


class zhidaoItem(scrapy.Item):
    qId = scrapy.Field()
    qTitle = scrapy.Field()
    qContent = scrapy.Field()
    qTag = scrapy.Field()
    qUrl = scrapy.Field()
    aId = scrapy.Field()
    aContent = scrapy.Field()
    aType = scrapy.Field()
