#!/bin/bash

hostlist="10.11.195.121 10.11.195.131 10.11.215.30 10.12.143.88 10.136.27.153 10.136.30.20"
for host in $hostlist
do
	ssh root@$host mkdir -p /search/liuyongshuai/srpp_new/data/
	ssh root@$host kill -9 $(ps aux|grep downServer|grep -v grep|awk '{print $2}')
	rsync $1 root@$host:/search/liuyongshuai/srpp_new/
	ssh root@$host nohup /search/liuyongshuai/srpp_new/downServer 11111 /search/liuyongshuai/srpp_new/down_server.ini > /dev/null 2>&1 &
done

