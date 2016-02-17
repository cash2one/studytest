#!/usr/local/bin/python
# -*- coding: utf-8 -*-

#####################################################
# 直接执行版
#####################################################

import sys
from Utils.action import *
reload(sys)
sys.setdefaultencoding('utf-8')



if __name__ == "__main__":
    if len(sys.argv) < 1:
        print "USAGE: %s task_id" % sys.argv[0]
        sys.exit()
    task_id = sys.argv[1]
    config_file = "./parser_client.ini"
    obj = action(task_id,config_file)

    #生成种子文件
    obj.gen_seed_file()

    #启动下载
    obj.start_download()

    #启动解析
    obj.start_parse()

    #启动入库
    obj.import_db()


