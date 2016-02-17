#!/usr/local/bin/python
# coding=utf-8


##############################################################################
# 对解析结果的几种处理方式，主要包括直接入库、rsync到指定机器上、直接返回
##############################################################################

import sys,os,time,simplejson,random
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
            return "invalid extra data"
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
        query = edata["query"]
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
        g_w = dict({
            "task_id":task_id,
            "engine":engine_id,
            "query":query,
        })
        sql = "DELETE FROM `tb_dcg_data_list` WHERE " + db.buildSQL("AND",**g_w)
        print sql
        db.execute(sql)

        #遍历结果并入库，先update根据affected_rows来决定是否插入
        for item in ret["list"]:
            rank = item["rank"]
            url = item["url"]
            raw_url = item["raw_url"]
            docid = item["docid"]
            title = item["title"]
            ad_type = item["ad_type"]
            if str(ad_type) not in ad_list:
                continue
            #拼凑SQL
            insert = dict({
                "task_id":task_id,
                "engine":engine_id,
                "query":query,
                "rank":rank,
                "url":url if len(url) > 0 else raw_url,
                "docid":docid,
                "title":title,
                "realurl":""
            })
            db.insert("tb_dcg_data_list",**insert)

        return True