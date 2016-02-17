#!/bin/bash

declare -i num=$(ps aux|grep downServer|grep -v grep|wc -l)
if [ $num -gt 0 ];then
	exit
fi

/search/liuyongshuai/srpp_new/downServer 11111 /search/liuyongshuai/srpp_new/down_server.ini > /dev/null &

