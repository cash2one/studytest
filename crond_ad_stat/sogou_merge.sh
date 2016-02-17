#!/bin/bash


list_dir=sogou_ad_data
flists=$(ls -1 ${list_dir}|grep list|grep -v done|grep -v tmp|grep -v grep)


for f in $flists
do
	list_file=${list_dir}/${f}
	python merge.py $list_file > /dev/null
done

