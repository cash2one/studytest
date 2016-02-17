#!/bin/bash
declare -i cur_run_num=$(ps aux|grep $0|grep -v grep|wc -l)
if [ $cur_run_num -gt 2 ];then
    exit
fi

down_dir=./sogou_ad_data/
src_file=sogouad.log
rsync_dest_dir=${down_dir}"%02d_%s_sogouad.log.2014-08-06"
bn=$(dirname $rsync_dest_dir)
if [ ! -d $bn ];then
    mkdir -p $bn
fi

room_list="sjs  zw  cnc"
declare -i rsync_cur_num=$(ps aux|grep sogou.expect|grep -v grep|wc -l)
declare -i rsync_max_concur_num=3

#download log file
declare -i num=1
declare -i all_files_num=42
for ((; $num<=14; num++))
do
    for room in $room_list
    do
        rsync_cur_num=$(ps aux|grep sogou.expect|grep -v grep|wc -l)
        while [ $rsync_cur_num -ge $rsync_max_concur_num ]
        do
            sleep 5
            rsync_cur_num=$(ps aux|grep sogou.expect|grep -v grep|wc -l)
        done
        dest_file=$(printf $rsync_dest_dir $num $room)
        if [ -f $dest_file ];then
            continue
        fi
        echo $dest_file
        expect sogou.expect $num $room $src_file $dest_file > /dev/null 2>&1 &
    done
done

cd $down_dir
declare -i num=$(ls -1|grep sogouad.log|grep -v utf8|grep -v done|grep -v tmp|grep -v grep|wc -l)
while [ $num -gt 0 ]
do
	sleep 5
	num=$(ls -1|grep sogouad.log|grep -v utf8|grep -v done|grep -v tmp|grep -v grep|wc -l)
done


