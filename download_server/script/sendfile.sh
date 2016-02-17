#!/bin/bash


if [ $# -lt 2 ];then
    echo "USAGE ./sendfile.sh local_file dest_file \n"
    exit
fi

local_file=$1
dest_file=$2

dest_dirname=$(dirname $dest_file)
ssh root@10.11.195.131 mkdir -p $dest_dirname

rsync -ar $local_file root@10.11.195.131:$dest_file

ret=$(rsync root@10.11.195.131:$dest_file)
if [ $ret -eq 0 ];then
	rm -f $local_file
else
	rsync -ar $local_file root@10.11.195.131:$dest_file
	rm -f $local_file
fi


