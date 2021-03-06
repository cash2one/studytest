# -*- coding: utf-8 -*-
import scrapy
import re, sys
from topword.items import topwordItem
from scrapy.selector import Selector
from scrapy.linkextractors import LinkExtractor
from scrapy.spiders import CrawlSpider, Rule

reload(sys)
sys.setdefaultencoding('utf-8')


class topwordSpider(CrawlSpider):
    name = "topword"
    allowed_domains = [
        "top.baidu.com",
        "top.so.com",
        "top.sogou.com",
        "www.zwbk.org",
        "www.baike.com"
    ]

    start_urls = [
        "http://top.baidu.com/boards",#百度热搜：全部榜单
        "http://top.sogou.com/home.html",#搜狗热搜：首页
        "http://top.so.com/",  # 360搜索热搜：首页
        "http://top.so.com/index.php?m=Hotnews&a=detail",  # 360搜索热搜：今日看点
        "http://top.so.com/index.php?m=Hotnews&a=detail&type=week_hot",  # 360搜索热搜：七日看点
        #"http://www.zwbk.org/MyTypeLayerMain.aspx?tid=001001",#中文百科在线
        #"http://www.zwbk.org/HtoLemma.aspx",#热门词条
        #"http://www.baike.com/cmsMorePage.do?page_now=1&templateId=100000030042",#互动百科：优质分类
    ]

    baidu_buzz_pattern = re.compile(r'^http:\/\/top\.baidu\.com\/buzz\?b=\d+')
    sogou_category_pattern = re.compile(r'^http:\/\/top\.sogou\.com\/\w+\/\w+_[\d]\.html')
    so_category_pattern1 = re.compile(r'^http:\/\/top\.so\.com\/index\.php\?page=[\d]{1,2}&tag=.*?&pid=.*?&c=.*?&m=.*')
    so_category_pattern2 = re.compile(r'^http:\/\/top\.so\.com\/index\.php\?m=.*?&c=.*')
    zwbk_tid_pattern = re.compile(r'^http:\/\/www\.zwbk\.org\/MyTypeLayer.*?\.aspx\?tid=.*')
    ciku5_pattern = re.compile(r'^http:\/\/www\.ciku5\.com\/words\?citype=\d+(&p=\d+)?')
    baike_category_pattern = re.compile(r'^http:\/\/www\.baike\.com\/cmsMorePage\.do\?page_now=\d.*?&templateId=.*')

    def parse(self, response):
        url = response.url
        if url.find("top.baidu.com") > 0:
            # 具体的热搜词页面
            words = response.xpath("//td[@class='keyword']/a[@class='list-title']/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item
            # 如果是榜单页面
            for href in response.css("div.all-list > div.bd > div.links > a::attr('href')"):
                url = response.urljoin(href.extract())
                if self.baidu_buzz_pattern.match(url):
                    yield scrapy.Request(url, callback=self.parse)
        elif url.find("top.sogou.com") > 0:
            # 具体的热搜词列表页面
            words = response.xpath("//li/span[@class='s2']/p[@class='p1']/a/text()|//li/span[@class='s2']/p[@class='p3']/a/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item
            # 如果是榜单页面
            for href in response.css("a::attr('href')"):
                url = response.urljoin(href.extract())
                if self.sogou_category_pattern.match(url):
                    yield scrapy.Request(url, callback=self.parse)
        elif url.find("top.so.com") > 0:
            # 如果是榜单页面
            for href in response.css("a::attr('href')"):
                url = response.urljoin(href.extract())
                if self.so_category_pattern1.match(url) or self.so_category_pattern2.match(url):
                    yield scrapy.Request(url, callback=self.parse)
            # 其他各分类热搜
            words = response.xpath("//div[@class='detail']/h3[@data-t='title']/a/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item
            # 今日热点热搜词
            words = response.xpath("//td[@class='rankitem__info']/a[@class='rankitem__name']/text()|//td[@class='rankitem__info']/div/a[@class='rankitem__name']/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item
        elif url.find("zwbk.org") > 0:
            for href in response.css("a::attr('href')"):
                url = response.urljoin(href.extract())
                if self.zwbk_tid_pattern.match(url):
                    yield scrapy.Request(url, callback=self.parse)
            words = response.xpath("//div[@class='sdiv2']/a[@target='_blank']/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item
            words = response.xpath("//div[@class='rdct_con']/a[@target='_blank']/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item
        elif url.find("www.baike.com") > 0:
            for href in response.css("a::attr('href')"):
                url = response.urljoin(href.extract())
                if self.baike_category_pattern.match(url):
                    yield scrapy.Request(url, callback=self.parse)
            words = response.xpath("//ul[@class='index_page']/li/a/text()").extract()
            for word in words:
                item = topwordItem()
                item["topword"] = word
                yield item