#!/bin/bash

src_dir=/search/liuyongshuai/task/
dest_task_dir=/search/liuyongshuai/task/
dest_ip=10.11.195.131
file_list=$(rsync root@${dest_ip}:${dest_task_dir}|grep task|awk '{print $NF}')

declare -i doing_num=0
for f in $file_list
do
	task_id=${f%%.task}
	doing_num=$(ps aux|grep $task_id|grep -v grep|wc -l)
	if [ $doing_num -gt 0 ];then
		continue
	fi
	desc_file=$(printf "root@%s:%s%s" $dest_ip $dest_task_dir $f)
	src_file=$(printf "%s%s" $src_dir $f)
	if [ -f $src_dir ];then
		continue
	fi
	rsync $desc_file $src_file
	log_file=$(printf "%s%s.log" $src_dir $task_id)
	python doAction.py $task_id > $log_file 2>&1 &
done


