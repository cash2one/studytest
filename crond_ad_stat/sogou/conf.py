#!/usr/local/bin/python
# -*- coding: utf-8 -*-

sogou_conf = dict({

	#渠道pid
	"sogou_channel_pid":"sogou-site-a5912d5771cbddba",

    #存放待解析的原始日志文件的目录
    "ad_log_data_dir":"/search/liuyongshuai/crond_ad_stat/sogou_ad_data",

    # 抽取html完毕后的列表目录文件
    # 格式：query、province、time、remoteIP、html_file
    "ad_log_list_dir":"/search/liuyongshuai/crond_ad_stat/sogou_ad_data/",

    #解析完毕后的结果文件，按日期分
    "result_file_dir":"/search/liuyongshuai/crond_ad_stat/sogou_ad_data/",

    #html文件的存放位置，%s后缀为日期，%s为md5(query) + uuid + .html后缀
    "html_file_dir":"/search/liuyongshuai/crond_ad_stat/html/sogou/%s",

    #解析结果的存放位置，%s后缀为日期，%s为md5(query) + uuid + .item后缀
    "item_file_dir":"/search/liuyongshuai/crond_ad_stat/items/sogou/",

    #所有统计数据的目录
    "stat_data_dir":"/search/liuyongshuai/crond_ad_stat/stat_data/",
})
