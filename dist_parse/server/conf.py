#!/usr/local/bin/python
# -*- coding: utf-8 -*-

engine_conf = dict({
	"sogou":"./conf/srpp-config-sogou.xml",
	"sogou-no-ad":"./conf/srpp-config-sogou-no-ad.xml",
    "sogou-wenwen":"./conf/srpp-config-sogou-wenwen.xml",
	"baidu":"./conf/srpp-config-baidu.xml",
	"360so":"./conf/srpp-config-360so.xml",
})

proxy_list = list([
    "adslspider01.web.zw.vm.sogou-op.org:8080",
    "adslspider02.web.zw.vm.sogou-op.org:8080",
    "adslspider03.web.zw.vm.sogou-op.org:8080",
    "adslspider04.web.zw.vm.sogou-op.org:8080",
])

parse_conf = dict({
	"tmp_dir":"./data",
	"logs_dir":"./logs",
})

db_conf = dict({
    "host":"10.11.195.131",
    "port":3306,
    "db":"mark_oss",
    "charset":"utf8",
    "user":"phpmyadmin",
    "passwd":"123456",
})

use_proxy = dict({
    "10.11.195.121":0,
    "10.13.195.121":0,

    "10.11.195.131":1,
    "10.13.195.131":1,

    "10.11.215.30":0,
    "10.13.215.30":0,

    "10.12.143.88":0,
    "10.14.143.88":0,

    "10.136.27.153":0,
    "10.146.27.153":0,

    "10.136.30.20":0,
    "10.146.30.20":0,
})
