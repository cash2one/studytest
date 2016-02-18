#!/bin/bash

#参数1：执行的JS文件
#参数2：要下载的URL
#参数3：使用的代理

if [ $# -lt 2 ];then
    echo "Invalid URL"
    exit 1;
fi
js=$1
url=$2
proxy=""
if [ $# -ge 3 ];then
    proxy="--proxy=$3"
fi
/usr/bin/phantomjs $proxy --output-encoding=gbk  --load-images=no $js $url
