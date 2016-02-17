#!/bin/bash

declare -i cur_num=$(ps aux|grep parseServer | grep -v grep|wc -l)
if [ $cur_num -lt 1 ];then
	python parseServer.py >> parseServer.log 2>&1 &
fi
