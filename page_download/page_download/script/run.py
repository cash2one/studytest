#!/usr/bin/python
# coding=gbk


############################################################################################
# ���������ļ�����ҳ�沢��⣬�����ҳ���Ƿ��Ǳ���ģ�����Ҫ������ץȡ
# *******ע���������̿���һֱ�����ţ������˳��Ļ����ؽ��̲����˳�*******
# argv[1]�������ţ������Ĺ����л������Ѵ������html�ļ�����־�������ظ�����
# argv[2]�������ļ����ҵ���������html�ļ��������ļ��������ҵ�task_id��query��engine_id
############################################################################################

import sys,os
from utils.utils import utils
from utils.parser_tool import parser_tool


if __name__ == "__main__":
    #�����Ĳ���У��
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
    #���������ļ�
    conf_info = utils.parseConf(conf_file)
    pt = parser_tool(task_id, conf_info)
    if not pt.check_task():
        sys.exit()
    pt.run()
