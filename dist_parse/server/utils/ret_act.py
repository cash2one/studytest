#!/usr/local/bin/python
# coding=utf-8


##############################################################################
# 对解析结果的几种处理方式，主要包括直接入库、rsync到指定机器上、直接返回
##############################################################################

import sys,os,time,simplejson,random
from urllib import quote,unquote
reload(sys)
sys.setdefaultencoding('utf-8')
from conf import parse_conf,db_conf
import uuid
from DB import DB
from utils import utils

class ret_act:

    ###############################################################
    # 用rsync方式传到指定机器上
    # extra：额外的数据，json串，send_dest等
    ###############################################################
    @staticmethod
    def rsync_ret(extra,**ret):
        edata = simplejson.loads(extra.strip())
        if not isinstance(edata,dict) or not edata.has_key("send_dest"):
            raise Exception("invalid extra data")
        final_ret = dict({"errno":0,"errmsg":ret})
        ostr = simplejson.dumps(final_ret)
        if not os.path.isdir(parse_conf["tmp_dir"]):
            os.makedirs(parse_conf["tmp_dir"],0755)
        #本地的临时文件名
        rand_str = str(time.time()) + utils.get_random_str(random.randrange(2, 20, 1))
        f = parse_conf["tmp_dir"].rstrip("/") + "/"
        f +=  str(uuid.uuid3(uuid.NAMESPACE_DNS, rand_str)) + ".html"
        fh = open(f,"a")
        fh.write(ostr)
        fh.close()
        send_dest = edata["send_dest"]
        os.system("(rsync %s %s ; rm -f %s) &" % (f,send_dest,f))
        return "success"

    ###############################################################
    # 用rsync方式传到指定机器上，如html文件发送到指定位置，解析结果也是
    # extra：额外的数据，json串，其字段如下
    # item_dest：解析的结果要发送到的地址
    # html_dest：源html文件要发送到的地址
    ###############################################################
    @staticmethod
    def rsync_ret_diff(html_content, extra, **ret):
        edata = simplejson.loads(extra.strip())
        if not isinstance(edata,dict) or not edata.has_key("item_dest") or not edata.has_key("html_dest"):
            raise Exception("invalid extra data")
        final_ret = dict({"errno":0,"errmsg":ret})
        ostr = simplejson.dumps(final_ret)
        if not os.path.isdir(parse_conf["tmp_dir"]):
            os.makedirs(parse_conf["tmp_dir"],0755)

        #先处理解析好的item文件
        rand_str = str(time.time()) + utils.get_random_str(random.randrange(2, 20, 1))
        f = parse_conf["tmp_dir"].rstrip("/") + "/"
        f +=  str(uuid.uuid3(uuid.NAMESPACE_DNS, rand_str)) + ".item"
        fh = open(f,"a")
        fh.write(ostr)
        fh.close()
        item_dest = edata["item_dest"]
        os.system("(rsync %s %s ; rm -f %s) &" % (f,item_dest,f))

        #再处理源html文件
        rand_str = str(time.time()) + utils.get_random_str(random.randrange(2, 20, 1))
        f = parse_conf["tmp_dir"].rstrip("/") + "/"
        f +=  str(uuid.uuid3(uuid.NAMESPACE_DNS, rand_str)) + ".html"
        fh = open(f,"a")
        fh.write(html_content)
        fh.close()
        html_dest = edata["html_dest"]
        os.system("(rsync %s %s ; rm -f %s) &" % (f,html_dest,f))

        return "success"

    ###############################################################
    # 用rsync方式传到指定机器上，如html文件发送到指定位置，解析结果也是
    # extra：额外的数据，json串，其字段如下
    # html_dest：源html文件要发送到的地址
    ###############################################################
    @staticmethod
    def rsync_ret_diff_diff(html_content, extra, **ret):
        edata = simplejson.loads(extra.strip())
        if not isinstance(edata,dict) or not edata.has_key("html_dest"):
            raise Exception("invalid extra data")

        if not os.path.isdir(parse_conf["tmp_dir"]):
            os.makedirs(parse_conf["tmp_dir"],0755)

        #再处理源html文件
        rand_str = str(time.time()) + utils.get_random_str(random.randrange(2, 20, 1))
        f = parse_conf["tmp_dir"].rstrip("/") + "/"
        f +=  str(uuid.uuid3(uuid.NAMESPACE_DNS, rand_str)) + ".html"
        fh = open(f,"a")
        fh.write(html_content)
        fh.close()
        html_dest = edata["html_dest"]
        os.system("(rsync %s %s ; rm -f %s) &" % (f,html_dest,f))

        return ret

    #########################################################################
    # 直接入库的方式
    # extra：包含入库所必需的字段，json格式，如query、task_id、engine_id、ad_type等
    #########################################################################
    @staticmethod
    def insert_db_ret(extra,**ret):
        edata = simplejson.loads(extra.strip())
        if not isinstance(edata,dict):
            raise Exception("invalid extra data")
        if "query" not in edata:
            raise Exception("not exists `query` in exrea")
        if "task_id" not in edata:
            raise Exception("not exists `task_id` in exrea")
        if "engine_id" not in edata:
            raise Exception("not exists `engine_id` in exrea")
        query = unquote(edata["query"])
        task_id = int(edata["task_id"])
        engine_id = int(edata["engine_id"])
        ad_type = edata["ad_type"]
        ad_list = ad_type.strip().split(",")
        if "0" not in ad_list:
            ad_list.append("0")
        if task_id <= 0 or engine_id <= 0:
            raise Exception("invalid task_id/engine_id")
        if len(query) <= 0:
            raise Exception("invalid length of query")

        #连接数据库
        db = DB(**db_conf)
        sql = "DELETE FROM `tb_dcg_data_list` WHERE `task_id`=%s AND `engine`=%s AND `query`=%s"
        print sql
        db.execute(sql,list([task_id,engine_id,query]))

        #要插入数据库的字段列表
        field_list = ["task_id","engine","query","rank","url","docid","title","realurl"]
        param = list()
        sql = "INSERT INTO `tb_dcg_data_list` (%s%s%s) VALUES " %("`","`,`".join(field_list),"`")
        for item in ret["list"]:
            rank = item["rank"]
            docid = item["docid"]
            ad_type = item["ad_type"]
            if str(ad_type) not in ad_list:
                continue
            try:
                title = item["title"].encode("utf8")
            except:
                title = ""
            url = item["url"].encode("utf8") if len(item["url"]) > 0 else item["raw_url"].encode("utf8")
            param.extend([task_id, engine_id, query, rank, url, docid, title, ""])
            sql = "%s (%s)," % (sql,("%s,"* len(field_list)).rstrip(","))

        return db.execute(sql.rstrip(","),param)