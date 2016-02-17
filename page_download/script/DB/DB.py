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
    # 从数据库里获取一条记录，是一个字典形式
    ####################################################
    def fetch_row(self,sql,param=list()):
        if not sql:
            raise TypeError,'invalid sql'
        try:
            self.query(sql,param)
        except Exception,args:
            print args
        return self.cursor.fetchone()


    ####################################################
    # 从数据库里获取一堆记录，返回一个元组，其元素为字典
    ####################################################
    def fetch_all(self,sql,param=list()):
        if not sql:
            raise TypeError,'invalid sql'
        try:
            self.query(sql,param)
        except Exception,args:
            print args
        return self.cursor.fetchall()

    ####################################################
    # 从数据库里获取一个字段的值
    ####################################################
    def fetch_col(self,sql,param=list()):
        if not sql:
            raise TypeError,'invalid sql'
        try:
            self.query(sql,param)
        except Exception,args:
            print args
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
        ds = self.buildSQL(",", **data)
        sql = "INSERT INTO `%s` SET %s" % (table,ds["sql"])

        #执行sql
        try:
            self.query(sql,ds["param"])
            insertid = self.conn.insert_id()
            self.conn.commit()
            return insertid
        except Exception,args:
            self.conn.rollback()
            raise args

    ####################################################
    # 执行一条UPDATE语句，返回影响行数
    # table：要操作的数据表名
    # where：拼装where条件用的
    # data：要更新的数据字典
    ####################################################
    def update(self, table,where,data):
        if not data or not table:
            raise Exception,'invalid data for update'

        #拼装data语句及where语句
        d_sql = self.buildSQL(",",**data)
        sql = "UPDATE `%s` SET %s " % (table,d_sql["sql"])
        if where and isinstance(where,dict):
            w_sql = self.buildSQL(" AND ",**where)
            sql = "%s WHERE %s" % (sql,w_sql["sql"])
            d_sql["param"].extend(w_sql["param"])

        #执行sql
        return self.execute(sql,d_sql["param"])

    ####################################################################
    # 执行一条INSERT.....ON DUPLICATE KEY UPDATE.....语句，返回影响行数
    # table：要操作的数据表名
    # insert_data：要插入的数据字典
    # update_data：当插入的key存在冲突时要更新的数据字典
    ####################################################################
    def insert_update(self, table, insert_data, update_data):
        if not insert_data or not table:
            raise Exception,'invalid data for insert'

        #拼装insert_data语句
        ins_s = self.buildSQL(",",**insert_data)
        up_s = self.buildSQL(",",**update_data)
        sql = "INSERT INTO `%s` SET %s ON DUPLICATE KEY UPDATE %s" % (
            table,ins_s["sql"],up_s["sql"]
        )
        ins_s["param"].extend(up_s["param"])

        #执行sql
        return self.execute(sql,ins_s["param"])

    ####################################################
    # 执行一条SQL语句，返回boolean型结果
    ####################################################
    def query(self,sql,param=list()):
        if not sql:
            raise Exception,'invalid sql for execute'

        #check connection
        try:
            self.conn.ping()
        except Exception,args:
            print args
            self._getConn()

        #execute SQL
        return self.cursor.execute(sql,param)

    ################################################################
    # 执行一条SQL语句，主要针对insert、update、delete等，返回影响行数
    ################################################################
    def execute(self,sql,param=list()):
        #execute SQL
        affected_rows = 0
        try:
            self.query(sql,param)
            affected_rows = self.conn.affected_rows()
            self.conn.commit()
        except Exception,args:
            print args

        return affected_rows

    ############################################################
    # 拼接SQL语句，如{'a':2,'b':'name'}输出字典：
    # {
    #       "sql":"`a`=%s,b=%s",
    #       "param":[a,b]
    # }
    # 默认以逗号分隔，也可为"AND"、"OR"等条件连接
    ############################################################
    def buildSQL(self, delim, **data):
        ret = dict()
        if not data:
            return ret

        sql = ""
        param = list()
        for key in data:
            sql += " `%s`=%s %s " % (key,"%s",delim)
            param.append(data[key])
        ret["sql"] = sql.strip(delim+" ")
        ret["param"] = param
        return ret

    def _getConn(self):
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
