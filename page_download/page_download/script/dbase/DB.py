#!/usr/bin/python
# coding=gbk

####################################################
# @author Liu Yongshuai<liuyongshuai@sogou-inc.com>
# ����������Դ����
####################################################


import sys,time
import MySQLdb
import MySQLdb.cursors


class DB(object):

    ####################################################
    # ��ȡMySQL���Ӿ�����������ӣ����ֵ�dbInfo��key���£�
    # host�����ݿ��ַ
    # user���û���
    # passwd������
    # port���˿ںţ�Ĭ��Ϊ3306
    # db�����ݿ�����
    # charset��ʹ�õı��룬Ĭ��ΪUTF8
    ####################################################
    def __init__(self, **dbInfo):
        self.conn = None
        self.cursor = None
        self.dbInfo = dbInfo

    ####################################################
    # �����ݿ����ȡһ����¼
    ####################################################
    def fetch_row(self,sql):
        if not sql:
            raise TypeError,'invalid sql'
        try:
            self.query(sql)
        except Exception,args:
            print args
            return False
        return self.cursor.fetchone()


    ####################################################
    # �����ݿ����ȡһ�Ѽ�¼
    ####################################################
    def fetch_all(self,sql):
        if not sql:
            raise TypeError,'invalid sql'
        try:
            self.query(sql)
        except Exception,args:
            print args
            return False
        return self.cursor.fetchall()

    ####################################################
    # �����ݿ����ȡһ���ֶε�ֵ
    ####################################################
    def fetch_col(self,sql):
        if not sql:
            raise TypeError,'invalid sql'
        try:
            self.query(sql)
        except Exception,args:
            print args
            return False
        ret = self.cursor.fetchone()
        if not ret or len(ret) <= 0:
            return False
        if isinstance(ret,dict):
            return ret.values()[0]
        else:
            return ret[0]

    ####################################################
    # ִ��һ��insert���
    ####################################################
    def insert(self, table, **data):
        if not data or not table:
            raise Exception,'invalid data for insert'

        ##ƴ��SQL
        sql = "INSERT INTO `" + table + "` SET "
        sql += self.buildSQL(",", **data)

        #ִ��sql
        insertid = 0
        try:
            self.query(sql)
            insertid = self.conn.insert_id()
            self.conn.commit()
            return long(insertid)
        except Exception,args:
            print args
        return long(insertid)

    ####################################################
    # ִ��һ��UPDATE��䣬����Ӱ������
    # table��Ҫ���������ݱ���
    # where��ƴװwhere�����õ�
    # data��Ҫ���µ������ֵ�
    ####################################################
    def update(self, table,where,data):
        if not data or not table:
            raise Exception,'invalid data for insert'

        sql = "UPDATE `%s` SET " + self.buildSQL(",",**data)
        if where and isinstance(where,dict):
            sql += " WHERE " + self.buildSQL("AND",**where)

        #ִ��sql
        affected_rows = 0
        try:
            affected_rows = self.execute(sql % table)
        except Exception,args:
            print args

        return affected_rows

    ####################################################################
    # ִ��һ��INSERT.....ON DUPLICATE KEY UPDATE.....��䣬����Ӱ������
    # table��Ҫ���������ݱ���
    # insert_data��Ҫ���������
    # update_data���������key���ڳ�ͻʱҪ���µ�����
    ####################################################################
    def insert_update(self, table, insert_data, update_data):
        if not insert_data or not table:
            raise Exception,'invalid data for insert'

        #ƴװSQL
        sql = "INSERT INTO %s SET " + self.buildSQL(",",**insert_data)
        sql += " ON DUPLICATE KEY UPDATE " + self.buildSQL(",",**update_data)

        #ִ��sql
        affected_rows = 0
        try:
            affected_rows = self.execute(sql % table)
        except Exception,args:
            print args

        return affected_rows

    ####################################################
    # ִ��һ��SQL��䣬����boolean�ͽ��
    ####################################################
    def query(self,sql):
        if not sql:
            raise Exception,'invalid sql for execute'

        #check connection
        try:
            if self.conn:
                self.conn.ping()
            else:
                self._getConn()
        except Exception,args:
            print args
            self._getConn()

        #execute SQL
        try:
            self.cursor.execute(sql)
            return True
        except Exception,args:
            print args
            return False

    ################################################################
    # ִ��һ��SQL��䣬��Ҫ���insert��update��delete�ȣ�����Ӱ������
    ################################################################
    def execute(self,sql):
        #execute SQL
        affected_rows = 0
        try:
            self.query(sql)
            affected_rows = self.conn.affected_rows()
            self.conn.commit()
        except Exception,args:
            print args

        return affected_rows

    ############################################################
    # ƴ��SQL��䣬��{'a':2,'b':'name'}�����`a`=2,`b`='name'
    # Ĭ���Զ��ŷָ���Ҳ��Ϊ"AND"��"OR"����������
    ############################################################
    def buildSQL(self, delim, **data):
        if not data:
            return ""

        sql = ""
        for key in data:
            sql += "`" + key + "`='" + MySQLdb.escape_string(str(data[key])) + "'" + delim

        return sql.rstrip(delim)

    def _getConn(self):
        try:
            self.conn = MySQLdb.connect(user=self.dbInfo["user"], \
                                        passwd=self.dbInfo["passwd"], \
                                        host=self.dbInfo["host"], \
                                        db=self.dbInfo["db"],\
                                        port=int(self.dbInfo["port"]),\
                                        charset=self.dbInfo["charset"], \
                                        cursorclass = MySQLdb.cursors.DictCursor)
            self.cursor = self.conn.cursor()
        except Exception,args:
            print args
            sys.exit()


if __name__ == "__main__":
    c = DB(user="phpmyadmin",passwd="123456",host="localhost",db="mark_oss",port=3306,charset="utf8")
    #print "----------fetch_row-------------"
    print c.fetch_row("select * from sortmonitor_query")
    print c.fetch_all("select * from sortmonitor_query limit 10")
    print c.fetch_col("select count(*) from sortmonitor_query")
    print c.insert("sortmonitor_query",query="test",url="http://www.baidu.com",vr_position="1")
    where=dict({"query_id":5613})
    data=dict({"query":"by_wendao","ctime":int(time.time())})
    print c.update("sortmonitor_query",where,data)
