# -*- coding: utf-8 -*-
import scrapy
import re, sys, os
from scrapy.selector import Selector
from qa.items import qaItem
import redis
from scrapy.linkextractors import LinkExtractor
from scrapy.spiders import CrawlSpider, Rule
from common.DB import DB
from common.config import mysql_host
from common.config import redis_host

reload(sys)
sys.setdefaultencoding('utf-8')


class qaSpider(CrawlSpider):
    name = "qa"
    allowed_domains = ["zhidao.baidu.com"]

    pattern = re.compile(r'^http:\/\/zhidao\.baidu\.com\/question\/\d+\.html.*$')
    t = re.compile(r'<.*?>')
    q = re.compile(r'<\/?(?:a|span).*?>')
    db = DB(**mysql_host["db_slave"])
    redis_pool = redis.ConnectionPool(**redis_host["slave"])

    def __init__(self, query = None, wfile = None):
        self.query = query
        self.file = wfile

    ################################################################
    # 处理搜索结果页面，抽取翻页的URL及单个问题详情页的URL
    ################################################################
    def parse(self, response):
        # 抓取列表
        for href in response.css("dl.dl > dt.dt > a.ti::attr('href')"):
            url = response.urljoin(href.extract())
            if self.pattern.match(url):
                qId = 0
                qUrlMatch = re.match(r'^http:\/\/zhidao\.baidu\.com\/question\/(\d+)\.html.*$', url)
                if qUrlMatch:
                    qId = qUrlMatch.group(1)

                # 检查该问题是否被抓取过
                r = redis.Redis(connection_pool=self.redis_pool)
                if not r.sismember("baidu_zhidao_qid", qId):
                    yield scrapy.Request(url, callback=self.parse_item)

        # 翻页操作
        for href in response.css("div.pager > a::attr('href')"):
            url = response.urljoin(href.extract())
            yield scrapy.Request(url, callback=self.parse)

    ################################################################
    # 问题详情页信息的抽取
    ################################################################
    def parse_item(self, response):
        # 问题标题
        qTitle = response.css("span.ask-title").extract()
        if qTitle:
            title = qTitle[0]
            tmp = self.t.split(title)
            qTitle = "".join(tmp)

        # 问题详情页的地址
        qUrl = response.url

        # 问题ID
        qUrlMatch = re.match(r'^http:\/\/zhidao\.baidu\.com\/question\/(\d+)\.html.*$', qUrl)
        if qUrlMatch:
            qId = qUrlMatch.group(1)

        # 问题的tag
        qTag = ""
        tag = response.xpath("//a[@class=\"question-tag-link\"]/text()").extract()
        if tag:
            for a in tag:
                qTag += "\t" + a

        # 问题的详细描述信息
        qContent = ""
        questions = response.xpath("//pre[@accuse=\"qContent\"]")
        for ques in questions:
            match = ques.re(r"<pre.*?>(.*?)<\/pre>")
            if match:
                tmp = match[0]
                tmp1 = self.q.split(tmp)
                qContent = "".join(tmp1)

        # 遍历抽取各个回答的信息
        answers = response.xpath("//pre[@accuse=\"aContent\"][@id]|//div[@accuse=\"aContent\"][@id]")
        for answer in answers:
            aContent = answer.extract()
            aContent = aContent.replace("\n", "")
            # 先匹配最佳、网友推荐的回答
            aMatch = re.match(r'<pre\s+id="(best-content|recommend-content)(.*?)".*?>(.*?)<\/pre>', aContent)
            # 最后匹配其他回答内容，一般都比较垃圾
            if not aMatch:
                aMatch = re.match(r'<div\s+id="(answer-content)(.*?)".*?>(.*?)<\/div>', aContent)
            if aMatch:
                item = qaItem()
                # 回答的类型：1被采纳的回答（最好的），2网友推荐的回答（次好的），3其他回答（较次的）
                item["aType"] = 0
                aType = aMatch.group(1)
                if aType == "best-content":
                    item["aType"] = 1
                elif aType == "recommend-content":
                    item["aType"] = 2
                elif aType == "answer-content":
                    item["aType"] = 3
                # 回答的ID
                item["aId"] = aMatch.group(2).strip("-")

                # 回答内容
                content = aMatch.group(3).strip(u"谢谢采纳").strip()
                tmp = self.q.split(content)
                item["aContent"] = "".join(tmp)

                # 有关问题的详细信息
                item["qTitle"] = qTitle
                item["qUrl"] = qUrl
                item["qId"] = qId
                item["qTag"] = qTag
                item["qContent"] = qContent

                yield item



                # 要搜索的问题：首先读取给定的文件，否则就用给定的关键词来搜索

    def start_requests(self):
        if self.file and os.path.isfile(self.file):
            fp = open(self.file, "r")
            for word in fp:
                query = word.strip()
                yield scrapy.Request("http://zhidao.baidu.com/search?word=%s" % query)
            fp.close()
        else:
            yield scrapy.Request("http://zhidao.baidu.com/search?word=%s" % self.query)