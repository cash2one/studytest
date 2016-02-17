# -*- coding: utf-8 -*-
import scrapy
import re,sys 
from scrapy.selector import Selector
from tech_163.items import Tech163Item
from scrapy.linkextractors import LinkExtractor
from scrapy.spiders import CrawlSpider,Rule
reload(sys)
sys.setdefaultencoding('utf-8')

class Tect163Spider(CrawlSpider):
	name = "tect163"
	allowed_domains = ["tech.163.com"]
	start_urls = ["http://tech.163.com/"]

	def parse(self,response):
		l = response.xpath("//h2[@class='color-link']/a/@href")
		for href in l:
			full_url = response.urljoin(href.extract())
			yield scrapy.Request(full_url,callback=self.parse_news);

	def parse_news(self,response):
		item = Tech163Item()
		item["news_thread"]=response.url.strip().split("/")[-1][:-5]
		title =response.xpath("/html/head/title/text()").extract()
		if title:
			item["news_title"] = (title[0][:-5]).encode("utf-8")

		source = response.xpath("//div[@class='ep-time-soure']/text()").extract()
		if source:
			item["news_time"] = (source[0])[:-5]

		news_from = response.xpath("//div[@class='ep-time-soure']/a[@id='ne_article_source']/text()").extract()
		if news_from:
			item["news_from"] = (news_from[0]).encode()

		from_url = response.xpath("//div[@class='ep-time-soure']/a[id='ne_article_source']/@href").extract()
		if from_url:
			item["from_url"]=from_url[0]

		news_body = response.xpath("//div[@id='endText']").extract()
		if news_body:
			item["news_body"] = (news_body[0]).decode().encode("utf-8","ignore")

		news_url=response.url
		if news_url:
			item["news_url"] = news_url
		yield item

