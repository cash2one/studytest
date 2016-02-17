#!/usr/local/bin/python
# -*- coding: utf-8 -*-

####################################################
# @author Liu Yongshuai<liuyongshuai@sogou-inc.com>
# 操作数据资源基类
####################################################


import sys,time
import MySQLdb
import MySQLdb.cursors


class DB(object):

    ####################################################
    # 获取MySQL连接句柄（不分主从），字典dbInfo各key如下：
    # host：数据库地址
    # user：用户名
    # passwd：密码
    # port：端口号，默认为3306
    # db：数据库名称
    # charset：使用的编码，默认为UTF8
    ####################################################
    def __init__(self, **dbInfo):
        self.conn = None
        self.cursor = None
        self.dbInfo = dbInfo
        self._getConn()

    ####################################################
    # 从数据库里获取一条记录
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
    # 从数据库里获取一堆记录
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
    # 从数据库里获取一个字段的值
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
    # 执行一条insert语句
    ####################################################
    def insert(self, table, **data):
        if not data or not table:
            raise Exception,'invalid data for insert'

        ##拼凑SQL
        sql = "INSERT INTO `" + table + "` SET "
        sql += self.buildSQL(",", **data)

        #执行sql
        insertid = 0
        try:
            self.query(sql)
            insertid = self.conn.insert_id()
            self.conn.commit()
            return long(insertid)
        except Exception,args:
            print args
			return 0
        return long(insertid)

    ####################################################
    # 执行一条UPDATE语句，返回影响行数
    # table：要操作的数据表名
    # where：拼装where条件用的
    # data：要更新的数据字典
    ####################################################
    def update(self, table,where,data):
        if not data or not table:
            raise Exception,'invalid data for update'

        sql = "UPDATE `%s` SET " + self.buildSQL(",",**data)
        if where and isinstance(where,dict):
            sql += " WHERE " + self.buildSQL("AND",**where)

        #执行sql
        affected_rows = 0
        try:
            affected_rows = self.execute(sql % table)
        except Exception,args:
            print args
			return 0

        return affected_rows

    ####################################################################
    # 执行一条INSERT.....ON DUPLICATE KEY UPDATE.....语句，返回影响行数
    # table：要操作的数据表名
    # insert_data：要插入的数据
    # update_data：当插入的key存在冲突时要更新的数据
    ####################################################################
    def insert_update(self, table, insert_data, update_data):
        if not insert_data or not table:
            raise Exception,'invalid data for insert'

        #拼装SQL
        sql = "INSERT INTO %s SET " + self.buildSQL(",",**insert_data)
        sql += " ON DUPLICATE KEY UPDATE " + self.buildSQL(",",**update_data)

        #执行sql
        affected_rows = 0
        try:
            affected_rows = self.execute(sql % table)
        except Exception,args:
            print args
			return 0

        return affected_rows

    ####################################################
    # 执行一条SQL语句，返回boolean型结果
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
    # 执行一条SQL语句，主要针对insert、update、delete等，返回影响行数
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
    # 拼接SQL语句，如{'a':2,'b':'name'}输出：`a`=2,`b`='name'
    # 默认以逗号分隔，也可为"AND"、"OR"等条件连接
    ############################################################
    def buildSQL(self, delim, **data):
        if not data:
            return ""

        sql = ""
        for key in data:
            sql += "`" + key + "`='" + self.conn.escape_string(str(data[key])) + "'" + delim

        return sql.rstrip(delim)

    def _getConn(self):
        try:
            if "charset" not in self.dbInfo:
                self.dbInfo["charset"] = None
            self.conn = MySQLdb.connect(user=self.dbInfo["user"], \
                                        passwd=self.dbInfo["passwd"], \
                                        host=self.dbInfo["host"], \
                                        db=self.dbInfo["db"],\
                                        port=int(self.dbInfo["port"]),\
                                        charset=self.dbInfo["charset"], \
                                        cursorclass=MySQLdb.cursors.DictCursor)
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
