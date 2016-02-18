# -*- coding: utf-8 -*-

################################################################
#  通用的工具类，全是静态方法
################################################################

import sys,os
reload(sys)
sys.setdefaultencoding('utf-8')

class utils:


    #######################################
    #获取本地的IP地址
    #######################################
    @staticmethod
    def get_local_ip():
        cmd = "/sbin/ifconfig|grep \"inet addr:\"|grep -v \"127.0.0.1\"|grep \"10\.\"|cut -d: -f2|awk '{print $1}'"
        r = os.popen(cmd)
        info = r.readline()
        info = info.strip('\r\n')
        return info


    #######################################
    #确保指定文件所在的目录一定存在，否则就创建目录，失败就返回False
    #######################################
    @staticmethod
    def mk_sure_dir(f):
        if os.path.isfile(f):
            return True
        dn = os.path.dirname(f)
        if not os.path.isdir(dn):
            return os.makedirs(dn,0755)
        return True
