# Define here the models for your scraped items
#
# See documentation in:
# http://doc.scrapy.org/topics/items.html

from scrapy.item import Item, Field
import scrapy

class Tech163Item(Item):
	news_thread=Field()    
	news_title=Field()    
	news_url=Field()    
	news_time=Field()    
	news_from=Field()    
	from_url=Field()    
	news_body=Field()    
