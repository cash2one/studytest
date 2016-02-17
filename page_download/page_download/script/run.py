#!/usr/bin/python
# coding=gbk


############################################################################################
# 给定种子文件解析页面并入库，并检查页面是否是被封的，否则还要再启动抓取
# *******注：解析进程可以一直运行着，它不退出的话下载进程不得退出*******
# argv[1]：任务编号：解析的过程中会生成已处理过的html文件的日志，以免重复解析
# argv[2]：配置文件：找到待解析的html文件及种子文件，即可找到task_id、query、engine_id
############################################################################################

import sys,os
from utils.utils import utils
from utils.parser_tool import parser_tool


if __name__ == "__main__":
    #基本的参数校验
    if len(sys.argv) < 3:
        sys.stderr.write("USAGE: <task_id>  <config_file>\n")
        sys.exit()
    task_id = int(sys.argv[1])
    if task_id <= 0:
        sys.stderr.write("invalid task_id, pls check and try \n")
        sys.exit()
    conf_file = sys.argv[2]
    if not os.path.isfile(conf_file):
        sys.stderr.write("invalid config file \n")
        sys.exit()
    #解析配置文件
    conf_info = utils.parseConf(conf_file)
    pt = parser_tool(task_id, conf_info)
    if not pt.check_task():
        sys.exit()
    pt.run()
