#!/bin/bash
declare -i cur_run_num=$(ps aux|grep $0|grep -v grep|wc -l)
if [ $cur_run_num -gt 2 ];then
    exit
fi

sleep 2
log_data_dir="sogou_ad_data"
declare -i retry_times=3
declare -i counter=0
while [ $counter -lt $retry_times ]
do
    if [ -d $log_data_dir ];then
        break
    else
        counter++
        sleep 5
    fi
done

#convert code to UTF-8
cd $log_data_dir
declare -i wait_file_num=0
declare -i is_doing=0
declare -i log_file_size=0
declare -i iconv_max_num=4
declare -i cur_concurr_iconv_num=0
declare -i all_files_num=42

###########################################################################
# 开始转码
###########################################################################
while [ 1 -gt 0 ]
do
	#检测文件总数，够了就退出
	wait_file_num=$(ls -1|grep utf8|grep -v done|grep -v tmp|grep -v grep|wc -l)
	if [ $wait_file_num -ge $all_files_num ];then
		break
	else
		sleep 10
	fi
	flist=$(ls -1|grep "sogouad.log"|grep -v utf8|grep -v grep)
	for log in $flist
	do
		#并发数超限
		cur_concurr_iconv_num=$(ps aux|grep iconv|grep "UTF-8"|grep -v sogou_iconv|grep -v grep|wc -l)
        echo $cur_concurr_iconv_num
		while [ $cur_concurr_iconv_num -ge $iconv_max_num ]
		do
			sleep 5
		    cur_concurr_iconv_num=$(ps aux|grep iconv|grep "UTF-8"|grep -v sogou_iconv|grep -v grep|wc -l)
		done
		#正在处理的
		is_doing=$(ps aux|grep $log|grep -v grep|wc -l)
		if [ $is_doing -gt 0 ];then
			continue
		fi
		#处理过的
		log_file_size=$(du -b $log|awk '{print $1}')
		if [ $log_file_size -le 0 ];then
			continue
		fi
		#转码，成功后将源文件置空
		tmp_utf8=$log.utf8.tmp
		utf8_file=$log.utf8
		if [ -f $utf8_file ];then
		    continue
		fi
		(iconv -f GB18030 -t UTF-8 $log > $tmp_utf8  && mv $tmp_utf8 $utf8_file && rm -f $log)  &
	done
done



