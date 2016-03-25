#!/bin/bash
cd /data/baiduZhidaoCrawler/seedword
scrapy crawl seedword
sleep 15
cd /data/baiduZhidaoCrawler/qa
kill -9 $(ps aux|grep crawl|grep -v grep| awk '{print $2}') 
kill -9 $(ps aux|grep crawl|grep -v grep| awk '{print $2}') 
killall scrapy
killall scrapy
killall scrapy
killall scrapy
killall scrapy
killall scrapy
sleep 15

scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &
scrapy crawl qa > /dev/null 2>&1 &


