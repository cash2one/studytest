#!/usr/bin/python
# coding=gbk

####################################################
# @author Liu Yongshuai<liuyongshuai@sogou-inc.com>
# ����������Դ����
####################################################

from dbase.DB import DB
import MySQLdb

class mark_oss:
    def __init__(self, **dbInfo):
        self.db = DB(**dbInfo)

    ##########################################################################################
    # ��ȡ�����������Ϣ
    ##########################################################################################
    def get_task_info(self,task_id):
        tid = str(int(task_id))
        sql = "SELECT * FROM `tb_dcg_task` WHERE `tid`=" + tid
        return self.db.fetch_row(sql)

    ##########################################################################################
    # ��ȡ������Ϣ�������ֵ���ʽ
    # @param    string  eid_str   �Զ��Ÿ�����eid��
    # @return   dict    ���ص�keyΪint
    ##########################################################################################
    def get_engine_list(self, eid_str):
        ret = dict()
        if len(eid_str) <= 0:
            return ret
        sql = "SELECT * FROM `tb_engine` WHERE `eid` IN ("
        el = eid_str.split(",")
        for eid in el:
            sql += str(int(eid)) + ","
        sql = sql.rstrip(",")
        sql += ")"
        tmp_list = self.db.fetch_all(sql)
        if len(tmp_list) <= 0:
            return ret
        for info in tmp_list:
            ret[int(info["eid"])] = info
        return ret

    ##########################################################################################
    # �������Ľ�����뵽tb_dcg_data_list����ȥ
    # ��Ҫ�ֶΰ�����task_id��query��url��docid��rank��engine��realurl��
    ##########################################################################################
    def insert_dcg_data_list(self, **data):
        return self.db.insert("tb_dcg_data_list", **data)

    ##########################################################################################
    # ƴ�ն���SQL��䣬���tb_dcg_data_list���
    # data = [{"id":"3","name":"wendao".....},{},....]
    ##########################################################################################
    def build_mutil_sql(self, data):
        sql = ""
        if len(data) <= 0:
            return sql
        if not isinstance(data[0],dict):
            return sql
        tmp = data[0].keys()
        if len(tmp) <= 0:
            return sql
        key_tmp = "`,`".join(tmp)
        sql = "INSERT INTO `tb_dcg_data_list` (`" + key_tmp + "`) VALUES "
        for k in range(len(data)):
            sql += "('"
            tmp_list = list()
            for key in tmp:
                tmp_list.append(MySQLdb.escape_string(str(data[k][key])))
            if len(tmp_list) != len(tmp):
                continue
            sql += "','".join(tmp_list) + "'), "
        sql = sql.rstrip()
        return sql.rstrip(",") + ";"