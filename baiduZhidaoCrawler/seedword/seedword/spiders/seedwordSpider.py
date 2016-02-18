# -*- coding: utf-8 -*-
import scrapy
import re,sys
from seedword.items import seedwordItem
from scrapy.selector import Selector
from scrapy.linkextractors import LinkExtractor
from scrapy.spiders import CrawlSpider,Rule
reload(sys)
sys.setdefaultencoding('utf-8')


class seedwordSpider(CrawlSpider):
    name = "seedword"
    allowed_domains = ["top.baidu.com","top.haosou.com","top.sogou.com"]

    start_urls = [
        "http://top.baidu.com/buzz?b=1&c=513&fr=topbuzz_b11_c513",#百度：实时热点
        "http://top.baidu.com/buzz?b=341&c=513&fr=topbuzz_b1_c513",#百度：今日热点
        "http://top.baidu.com/buzz?b=42&c=513&fr=topbuzz_b341_c513",#百度：七日热点
        "http://top.baidu.com/buzz?b=342&c=513&fr=topbuzz_b42_c513",#百度：民生热点
        "http://top.baidu.com/buzz?b=344&c=513&fr=topbuzz_b342_c513",#百度：娱乐热点
        "http://top.baidu.com/buzz?b=11&c=513&fr=topbuzz_b344_c513",#百度：体育热点
        "http://top.sogou.com/hotword0.html",#搜狗：最近10天热点
        "http://top.sogou.com/hotword1.html",
        "http://top.sogou.com/hotword2.html",
        "http://top.sogou.com/hotword3.html",
        "http://top.haosou.com/index.php?m=Hotnews&a=detail",#好搜：今日热点
        "http://top.haosou.com/index.php?m=Hotnews&a=detail&type=week_hot",#好搜：七日看点
    ]


    def parse(self,response):
        url = response.url
        if url.find("top.baidu.com") > 0:
            words = response.xpath("//td[@class='keyword']/a[@class='list-title']/text()").extract()
            for word in words:
                item = seedwordItem()
                item["seedword"] = word
                yield item
        elif url.find("top.sogou.com") > 0:
            words = response.xpath("//div[@class='hotlist']/ul/li/a/text()").extract()
            for word in words:
                item = seedwordItem()
                item["seedword"] = word
                yield item
        elif url.find("top.haosou.com") > 0:
            words = response.xpath("//tr[@class='rankitem']/td[@class='rankitem__info']/a[class='rankitem__name']/text()").extract()
            for word in words:
                item = seedwordItem()
                item["seedword"] = word
                yield item

