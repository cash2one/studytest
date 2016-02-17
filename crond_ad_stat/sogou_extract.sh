#!/bin/bash
declare -i cur_run_num=$(ps aux|grep $0|grep -v grep|wc -l)
if [ $cur_run_num -gt 2 ];then
    exit
fi

log_data_dir="sogou_ad_data"

#convert code to UTF-8
declare -i is_doing=0
declare -i iconv_max_num=4
declare -i cur_concurr_iconv_num=0
declare -i all_files_num=42
while [ 1 -gt 0 ]
do
	#抽取之
	flist=$(ls -1 $log_data_dir|grep "sogouad.log"|grep utf8|grep -v done|grep -v tmp|grep -v grep)
	for log in $flist
	do
		log_file=${log_data_dir}/${log}
		log_file_done=${log_file}.done
		#并发数超限
		cur_concurr_iconv_num=$(ps aux|grep extract.py|grep -v grep|wc -l)
        echo $cur_concurr_iconv_num
		while [ $cur_concurr_iconv_num -ge $iconv_max_num ]
		do
			sleep 5
		    cur_concurr_iconv_num=$(ps aux|grep extract.py|grep -v grep|wc -l)
		done
		#正在处理的
		is_doing=$(ps aux|grep extract|grep $log|grep -v grep|wc -l)
		if [ $is_doing -gt 0 ];then
			continue
		fi
		#处理过的
		if [ -f $log_file_done ];then
			continue
		fi
		python extract.py $log_file > /dev/null  &
	done
	#检测文件总数，够了就退出
	wait_file_num=$(ls -1 $log_data_dir|grep utf8|grep done|grep -v grep|wc -l)
	if [ $wait_file_num -ge $all_files_num ];then
		break
	fi
done





