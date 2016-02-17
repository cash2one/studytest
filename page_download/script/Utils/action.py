#!/usr/local/bin/python
# -*- coding: utf-8 -*-

#####################################################
# 生成种子文件、开始下载、发送解析请求、入库操作等等
#####################################################

from utils import *
from DB.DB import *
import hashlib,os,sys,simplejson
from urllib import quote,unquote
reload(sys)
sys.setdefaultencoding('utf-8')

class action:
    def __init__(self,task_id,config_file):
        self.task_id = task_id
        self.db = None
        self.conf = Utils.parseConfig(config_file)
        self.config_file = config_file
        self.task_info = dict()
        self.engine_list = dict()
        self.seed_file = "%s/%s.seed" % (self.conf["task_dir"].rstrip(), str(self.task_id))
        Utils.mk_sure_dir(self.seed_file)
        self.item_dir = list()

    ##########################################################################################
    # 生成种子文件，其格式为：url、local_html_file、remote_html_file、query
    ##########################################################################################
    def gen_seed_file(self):
        #任务及引擎信息
        self.__get_engine_list__()

        #创建任务的日期
        timeArray = time.localtime(int(self.task_info["createtime"]))
        year_month = time.strftime("%Y-%m", timeArray)

        #query列表
        sql = "SELECT * FROM `tb_dcg_query` WHERE `tid`=%s"
        ret = self.db.fetch_all(sql,[self.task_id])
        if not ret:
            print "get task query list failed"
            sys.exit()

        #种子文件格式：url、local_html_file、remote_html_file、query
        eng_list = self.task_info["engine"].split("\t")
        fp = open(self.seed_file, "w")
        for q in ret:
            query = q["query"].encode("utf8")
            for eid in eng_list:
                eid = eid.encode("utf8")
                eurl = self.engine_list[eid]["url"].encode("utf8")
                url = eurl.replace("__QUERY__", query)
                #remote html file
                remote_html_file = "%s/%s/%s_%s/%s.html" % (
                    self.conf["remote_snapshot_dir"].rstrip("/"), year_month, str(self.task_id), str(eid),
                    hashlib.md5(query).hexdigest()
                )
                #local html file
                local_html_file ="%s/%s_%s/%s.html" % (
                    self.conf["local_snapshot_dir"].rstrip("/"),str(self.task_id),
                    str(eid),hashlib.md5(query).hexdigest()
                )
                Utils.mk_sure_dir(local_html_file)
                #写入种子文件
                ostr = "%s\t%s\t%s\t%s\n" % (
                    quote(url,":?=/&"), local_html_file, remote_html_file,str(quote(query))
                )
                #本地的item dir，确保它存在
                item_file = "%s/%s_%s/%s.item" % (
                    self.conf["task_dir"].rstrip(),self.task_id,str(eid),hashlib.md5(query).hexdigest()
                )
                Utils.mk_sure_dir(item_file)
                fp.write(ostr)
                fp.flush()
        fp.close()
        return True

    ##########################################################################################
    # 开始启动下载进程
    ##########################################################################################
    def start_download(self):
        cmd = "kill -9 $(ps aux|grep download|grep %s|grep -v grep|awk '{print $2}')" % (self.task_id)
        print cmd
        os.system(cmd)
        cmd = "%s %s &" % (self.conf["download"],self.seed_file)
        print cmd
        os.system(cmd)
        return True

    ##########################################################################################
    # 开始启动解析进程
    ##########################################################################################
    def start_parse(self):
        cmd = "kill -9 $(ps aux|grep %s|grep %s|grep -v grep|akw '{print $2}')" % (
            self.conf["parse"],self.seed_file
        )
        print cmd
        os.system(cmd)
        cmd = "%s %s %s" % (
            self.conf["parse"],self.seed_file,self.config_file
        )
        print cmd
        os.system(cmd)
        return True

    ##########################################################################################
    # 解析好的结果导入数据库
    ##########################################################################################
    def import_db(self):
        fp = open(self.seed_file,"r")
        for line in fp:
            item = line.strip().split("\t")
            if len(item) < 4:
                continue
            html_file = item[1]
            query = unquote(item[3])
            ele1 = os.path.basename(os.path.dirname(html_file))
            ele2 = os.path.basename(html_file)[:-5] + ".item"
            item_file = "%s/%s/%s" % (self.conf["task_dir"].rstrip(),ele1,ele2)
            if not os.path.isfile(item_file):
                continue
            print "start insert %s to db" % query
            self.__insert_db__(item_file,query)
        fp.close()

        #更改任务的状态为抓取成功
        data = dict({"status":3})
        cond = dict({"tid":self.task_id})
        self.db.update("tb_dcg_task",cond,data)
        return True

    ##########################################################################################
    # 获取当前任务所使用的引擎列表
    ##########################################################################################
    def __get_engine_list__(self):
        if self.db == None:
            self.__connect_db__()
        if len(self.task_info) <= 0:
            self.__get_task_info__()
        eng_list = self.task_info["engine"].split("\t")
        sql = "SELECT * FROM `tb_engine` WHERE `eid` IN (%s)" % ("%s,"*len(eng_list)).rstrip(",")
        ret = self.db.fetch_all(sql,eng_list)
        if not ret:
            print "get engine list failed"
            sys.exit()
        for elem in ret:
            self.engine_list[str(elem["eid"])] = dict({
                    "url":elem["url"].encode("utf8"),
                    "name":elem["name"].encode("utf8")
                    })
        return True

    ##########################################################################################
    # 获取任务的详细信息
    ##########################################################################################
    def __get_task_info__(self):
        if self.db == None:
            self.__connect_db__()
        sql = "SELECT * FROM `tb_dcg_task` WHERE `tid`=%s"
        self.task_info = self.db.fetch_row(sql,[self.task_id])
        return True

    ##########################################################################################
    # 连接数据库操作
    ##########################################################################################
    def __connect_db__(self):
        self.db = DB(
            host=self.conf["mysql_host"],
            user=self.conf["mysql_user"],
            passwd=self.conf["mysql_password"],
            port=int(self.conf["mysql_port"]),
            db=self.conf["mysql_db"],
            charset=self.conf["mysql_charset"],
        )
        return True

    ##########################################################################################
    # 单个html的结果入库操作
    ##########################################################################################
    def __insert_db__(self,item_file,query):
        item_json = open(item_file,"r").read()
        item_json = item_json.strip()
        try:
            items = simplejson.loads(item_json,strict=False)
        except Exception,args:
            print args
            os.remove(item_file)
            return False
        #解析状态不对
        if not items.has_key("errno") or int(items["errno"]) != 0 or not items.has_key("errmsg"):
            os.remove(item_file)
            return False
        #解析结果不对
        if len(items["errmsg"]) <= 0:
            return False
        #提取task_id和engine_id
        tid_eid = os.path.basename(os.path.dirname(item_file))
        tmp = tid_eid.split("_")
        if len(tmp) != 2:
            os.remove(item_file)
            return False
        tid = tmp[0]
        eid = tmp[1]
        if tid <= 0 or eid <= 0:
            return False
        ret_list = items["errmsg"]["list"]
        if len(ret_list) <= 0:
            return False

        #删除旧有数据
        sql = "DELETE FROM `tb_dcg_data_list` WHERE `task_id`=%s AND `engine`=%s AND `query`=%s"
        self.db.execute(sql,[tid,eid,query])

        #要插入数据库的字段列表
        field_list = ["task_id","engine","query","rank","url","docid","title","realurl"]
        param = list()
        sql = "INSERT INTO `tb_dcg_data_list` (%s%s%s) VALUES " %("`","`,`".join(field_list),"`")
        #再遍历解析好的结果
        for item in ret_list:
            rank = item["rank"]
            url = item["url"] if len(item["url"]) > 0 else item["raw_url"]
            docid = item["docid"]
            try:
                title = item["title"].encode("utf8")
            except:
                title = ""
            param.extend([tid, eid, query, rank, url, docid, title, ""])
            sql = "%s (%s)," % (sql,("%s,"* len(field_list)).rstrip(","))

        return self.db.execute(sql.rstrip(","),param)