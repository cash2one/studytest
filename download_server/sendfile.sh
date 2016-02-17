#!/bin/bash


if [ $# -lt 2 ];then
    echo "USAGE ./sendfile.sh local_file dest_file \n"
    exit
fi

local_file=$1
dest_file=$2

dest_dirname=$(dirname $dest_file)
ssh root@10.11.195.131 mkdir -p $dest_dirname

rsync --remove-source-files -ar $local_file root@10.11.195.131:$dest_file


