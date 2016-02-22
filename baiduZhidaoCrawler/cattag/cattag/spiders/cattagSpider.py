# -*- coding: utf-8 -*-
import scrapy
import re, sys
from scrapy.selector import Selector
from scrapy.linkextractors import LinkExtractor
from urlparse import urlparse, parse_qs
from scrapy.spiders import CrawlSpider, Rule
from common.DB import DB
from common.config import mysql_host
from urllib import unquote
import HTMLParser

reload(sys)
sys.setdefaultencoding('utf-8')


class cattagSpider(CrawlSpider):
    name = "cattag"
    allowed_domains = ["zhidao.baidu.com"]

    html_parser = HTMLParser.HTMLParser()

    start_urls = ["http://zhidao.baidu.com/list"]
    strip = re.compile(r'<.*?>')

    def parse(self, response):
        referer = response.url
        item = urlparse(referer)
        info = parse_qs(item.query, True)
        parent_cid = 0
        if "cid" in info:
            parent_cid = info["cid"][0]
        for tag in response.css("li.content-list-item-0 > div.content-list-item-tags  a.tag-item"):
            tlink = tag.extract()
            tlink = tlink.decode("gbk", "ignore").encode("utf8")
            tlink = unquote(tlink)
            tlink = self.html_parser.unescape(tlink)
            qUrlMatch = re.match(r'<a.*?href="(.*?)".*?>(.*?)<\/a>', tlink, re.M | re.S | re.I)
            if qUrlMatch:
                url = qUrlMatch.group(1)
                url = response.urljoin(url)
                url_item = urlparse(url)
                arg_info = parse_qs(url_item.query, True)
                cid = arg_info["cid"][0]
                d = DB(**mysql_host["db_master"])
                if "tag" in arg_info:
                    tagname = arg_info["tag"][0].strip()
                    d.insert("tag", tag=tagname, cid=cid)
                    d.insert("seedword", word=unquote(tagname))
        # 抓取列表
        for tag in response.css("li.view-list-item > div.tag-content  a"):
            tlink = tag.extract()
            tlink = unquote(tlink)
            tlink = self.html_parser.unescape(tlink)
            qUrlMatch = re.match(r'<a.*?href="(.*?)".*?>(.*?)<\/a>', tlink, re.M | re.S | re.I)
            if qUrlMatch:
                url = qUrlMatch.group(1)
                url = response.urljoin(url)
                url_item = urlparse(url)
                arg_info = parse_qs(url_item.query, True)
                cid = arg_info["cid"][0]
                d = DB(**mysql_host["db_master"])
                if "tag" in arg_info:
                    tagname = arg_info["tag"][0].strip()
                    d.insert("tag", tag=tagname, cid=cid)
                    d.insert("seedword", word=unquote(tagname).strip())
                else:
                    cname = qUrlMatch.group(2)
                    tmp = self.strip.split(cname)
                    cname = "".join(tmp)
                    d.insert("category", cid=cid, cname=cname.strip(), parent_cid=parent_cid)
                    d.insert("seedword", word=unquote(cname).strip())
                yield scrapy.Request(url, callback=self.parse)