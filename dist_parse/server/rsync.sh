#!/bin/bash

f=$1
d=$2

hosts=""
hosts=${hosts}"10.11.195.121 "	
hosts=${hosts}"10.11.195.131 "
hosts=${hosts}"10.11.215.30 "
hosts=${hosts}"10.12.143.88 "
hosts=${hosts}"10.136.27.153 "
hosts=${hosts}"10.136.30.20 "
hosts=${hosts}"10.141.49.52 "

for h in $hosts
do
	rsync $f root@$h:$d
done
