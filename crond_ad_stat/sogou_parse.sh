#!/bin/bash

hosts=""
hosts=${hosts}"10.11.195.121:9090 "
hosts=${hosts}"10.11.195.121:9091 "
hosts=${hosts}"10.11.195.121:9092 "
hosts=${hosts}"10.11.195.131:9090 "
hosts=${hosts}"10.11.195.131:9091 "
hosts=${hosts}"10.11.195.131:9092 "
hosts=${hosts}"10.11.215.30:9090 "
hosts=${hosts}"10.11.215.30:9091 "
hosts=${hosts}"10.11.215.30:9092 "
hosts=${hosts}"10.12.143.88:9090 "
hosts=${hosts}"10.12.143.88:9091 "
hosts=${hosts}"10.12.143.88:9092 "
hosts=${hosts}"10.136.27.153:9090 "
hosts=${hosts}"10.136.27.153:9091 "
hosts=${hosts}"10.136.27.153:9092 "
hosts=${hosts}"10.136.30.20:9091 "
hosts=${hosts}"10.136.30.20:9092 "
hosts=${hosts}"10.136.30.20:9093 "
hosts=${hosts}"10.141.49.52:9090 "
hosts=${hosts}"10.141.49.52:9091 "
hosts=${hosts}"10.141.49.52:9092 "
hosts=${hosts}"10.141.49.52:9093 "
hosts=${hosts}"10.141.49.52:9094 "

flists=$(ls -1 sogou_ad_data|grep list|grep -v grep)

declare -i proc_limit=11
declare -i proc_num=0
declare -i all_proc_num=0
for f in $flists
do
	proc_num=0
	for h in $hosts
	do
		log_file=./logs/${f}_${h}.log
		#当前的host已经在解析其他任务了
		proc_num=$(ps aux|grep $h|grep -v grep|wc -l)
		if [ $proc_num -gt 0 ];then
			continue
		fi
		#当前的日志文件已经在处理了
		proc_num=$(ps aux|grep $f|grep -v grep|wc -l)
		if [ $proc_num -gt 0 ];then
			continue
		fi
		echo "start process $f $h"
		python parse.py $h ./sogou_ad_data/$f >  $log_file &
	done
	all_proc_num=$(ps aux|grep parse.py|grep list|grep -v grep|wc -l)
	while [ $all_proc_num -ge $proc_limit ]
	do
		sleep 5
		all_proc_num=$(ps aux|grep parse.py|grep sogou_ad_data|grep -v grep|wc -l)
	done
done

#解析完毕判断
all_proc_num=$(ps aux|grep parse.py|grep list|grep -v grep|wc -l)
while [ $all_proc_num -gt 0 ]
do
	sleep 60
	all_proc_num=$(ps aux|grep parse.py|grep list|grep -v grep|wc -l)
done




