#encoding:utf-8

import scrapy
import re 
from scrapy.selector import Selector
from tech163.items import Tech163Item
from scrapy.contrib.linkextractors.sgml import SgmlLinkExtractor
from scrapy.contrib.spiders import CrawlSpider,Rule

class Tech163Spider(CrawlSpider):
	name = "news"
	allowed_domains = ["tech.163.com"]
	start_urls = "http://tech.163.com/"
	rules = (
			Rule(SgmlLinkExtractor(allow=r"/14/08\d+/*"),callback="parse_news",follow=True),
	)

	def parse_news(self,response):
		item = Tech163Item()
		item["news_thread"]=response.url.strip().split("/")[-1][:-5]

		title =response.xpath("/html/head/title/text()").extract()
		if title:
			item["news_title"] = title[0][:-5]

		source = response.xpath("//div/[@class='left']/text()").extract()
		if source:
			item["news_time"] = source[0][:-5]

		news_from = response.xpath("//div[@class='left']/a/text()").extract()
		if news_from:
			item["news_from"] = news_from[0]

		from_url = response.xpath("//div[@class='left']/a/@href").extract()
		if from_url:
			item["from_url"]=from_url[0]

		news_body = response.xpath("//div[@id='endText']/p/text()").extract()
		if news_body:
			item["news_body"]=news_body

		news_url=response.url
		if news_url:
			item["news_url"] = news_url
		return item












