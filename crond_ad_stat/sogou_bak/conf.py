#!/usr/local/bin/python
# -*- coding: utf-8 -*-

from Queue import Queue

sogou_conf = dict({
    "parseThreadNum":10, #解析线程数
    "extractQueueObj":Queue(50),#存放待抽取的队列
    "parseQueueObj":Queue(50),#待解析队列
    "mergeQueueObj":Queue(50),#merge队列
	
	#渠道pid
	"sogou_channel_pid":"sogou-site-a5912d5771cbddba",

    #存放待解析的原始日志文件的目录
    "ad_log_data_dir":"/search/liuyongshuai/crond_ad_stat/sogou_ad_data",

    # 抽取html完毕后的列表目录文件
    # 格式：query、province、time、remoteIP、html_file
    "ad_log_list_dir":"/search/liuyongshuai/crond_ad_stat/sogou_ad_data/",

    #解析完毕后的结果文件，按日期分
    "result_file_dir":"/search/liuyongshuai/crond_ad_stat/sogou_ad_data/",

    #已解析完的文件日志，避免重复解析
    "done_log_file":"/search/liuyongshuai/crond_ad_stat/logs/sogou_done_%s.log",

    #html文件的存放位置，%s后缀为日期，%s为md5(query) + uuid + .html后缀
    "html_file_dir":"/search/liuyongshuai/crond_ad_stat/html/sogou/%s",

    #解析结果的存放位置，%s后缀为日期，%s为md5(query) + uuid + .item后缀
    "item_file_dir":"/search/liuyongshuai/crond_ad_stat/items/sogou/",

    #所有统计数据的目录
    "stat_data_dir":"/search/liuyongshuai/crond_ad_stat/stat_data/",

    #解析机器列表
    "parse_host":list([
        "10.11.195.121:9090",
		
        "10.11.195.131:9090",
		"10.11.195.131:9091",
		
        "10.11.215.30:9090",
		
        "10.12.143.88:9090",
		"10.12.143.88:9091",
		
        "10.136.27.153:9090",
		"10.136.27.153:9091",
		
        "10.136.30.20:9091",
		"10.136.30.20:9092",
		
        "10.141.49.52:9090",
    ]),
})
