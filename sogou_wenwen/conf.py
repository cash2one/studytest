#!/usr/local/bin/python
# -*- coding: utf-8 -*-

engine_conf = dict({
	"sogou":"./conf/srpp-config-sogou.xml",
	"sogou-no-ad":"./conf/srpp-config-sogou-no-ad.xml",
    "sogou-wenwen":"./conf/srpp-config-sogou-wenwen.xml",
	"baidu":"./conf/srpp-config-baidu.xml",
	"360so":"./conf/srpp-config-360so.xml",
})

parse_conf = dict({
	"tmp_dir":"./data",
	"logs_dir":"./logs",
})

db_conf = dict({
    "host":"10.11.195.131",
    "port":3306,
    "db":"mark_oss",
    "charset":"utf8",
    "user":"phpadmin",
    "passwd":"123456",
})

