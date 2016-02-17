# Scrapy settings for tech163 project
#
# For simplicity, this file contains only the most important settings by
# default. All the other settings are documented here:
#
#     http://doc.scrapy.org/topics/settings.html
#

BOT_NAME = 'tech163'
BOT_VERSION = '1.0'

SPIDER_MODULES = ['tech163.spiders']
NEWSPIDER_MODULE = 'tech163.spiders'
ITEM_PIPELINES=['tech163.pipelines.Tech163Pipeline']
USER_AGENT = 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0' 

DONWLOAD_TIMEOUT=15

