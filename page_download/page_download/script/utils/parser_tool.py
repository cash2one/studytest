#!/usr/bin/python
# coding=gbk

####################################################
# @author Liu Yongshuai<liuyongshuai@sogou-inc.com>
# 解析页面时的工具类，管理用的
####################################################

import sys,os,string,time
from dbase.mark_oss import mark_oss
from utils import utils
from webpage_parser import WebPageParser
from webpage_config import WebPageConfig
import logging
import logging.handlers
from urllib import unquote

class parser_tool:
    def __init__(self,task_id,conf_info):
        self.task_id = task_id #任务编号，目前绑死了
        self.conf = conf_info #解析出来的配置文件的信息
        self.engineList = dict() #所用到的引擎的详细信息
        self.task_info = dict() #任务的详细信息
        self.seed_fp = "" #种子文件
        self.parsed_fp= "" #已解析完成的html文件，避免重复解析
        self.mk = None #操作mark_oss库的类实例
        self.parsed_set = set() #已解析过的html的集合
        self.logger = "" #处理过程中的日志文件
        self.wp_parse = None #webpage_parser
        self.wp_confs = dict() #webpage_config
        self.sql_fp = "" #临时存储SQL语句的


    def run(self):
        #先来一次下载
        task_file = self.conf["TASK_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".task"
        os.system("/work/test_test/page_download/server/task %s" % task_file)

        #开始解析了
        self.__init_conf__()
        #是否继续？
        while 1:
            is_continue_loop = False
            self.seed_fp.seek(0)
            for src in self.seed_fp:
                item = src.strip(string.whitespace).split("\t")
                if len(item) < 3:
                    continue
                query = unquote(item[0])
                html_file = item[2]
                if not os.path.isfile(html_file):
                    is_continue_loop = True
                    self.logger.debug("html_file not exists: " + html_file)
                    continue
                self.__parse_html__(query,html_file)
            if not is_continue_loop:
                break
            os.system("/work/test_test/page_download/server/task %s" % task_file)
        return True

    ##########################################################################################
    #解析一个html文件的逻辑操作，包括入库、获取配置文件等
    ##########################################################################################
    def __parse_html__(self, query, html_file):
        if not os.path.isfile(html_file):
            return False
        if html_file in self.parsed_set:
            return True
        #获取引擎编号
        tmp_dir_name = os.path.dirname(html_file)
        tmp_base_name = os.path.basename(tmp_dir_name)
        if tmp_base_name.find("_") <= 0:
            return None
        eid = tmp_base_name[tmp_base_name.find("_")+1:]

        srpp_conf = self.__get_srpp_config_file__(html_file)
        if not os.path.isfile(srpp_conf):
            return False
        config = None
        if srpp_conf not in self.wp_confs:
            self.wp_confs[srpp_conf] = WebPageConfig(self.logger)
            if self.wp_confs[srpp_conf].ParseFile(srpp_conf) != 0:
                sys.stderr.write("parse config file failed %s\n" % srpp_conf)
        config = self.wp_confs[srpp_conf]

        #对html文件做简单处理
        fp = open(html_file,"r")
        html_content = fp.read()
        fp.close()
        #文件大小有问题，或是百度的需要输入验证码的页面
        if len(html_content) <= 0 or (utils.is_baidu_verifycode(html_content) and \
                        int(eid) in self.engineList and \
                        self.engineList[int(eid)]["name"].find("baidu")>=0):
            os.remove(html_file)
            return False
        html_content = utils.strip_tags(html_content)

        #开始解析
        self.wp_parse.Init(config)
        self.wp_parse.Reset()
        self.wp_parse.ParseHTML(html_content)
        self.wp_parse.RedirectUrl()

        #遍历及入库
        ret_sql = list()
        for index,result in enumerate(self.wp_parse.page_element_list):
            ret_data = dict({"task_id":str(self.task_id),"engine":eid,"query":query,"realurl":""})
            ret_data["rank"] = result.rank
            #index = result.index
            #parent_index = result.parent_index
            #children_num = len(result.children)
            #has_script = result.has_script
            #path = str(result.analyse_path)
            #node_info = result.node_info.encode('gbk','ignore')
            #type_string = str(result.type_str)
            #analyse_type_string = str(result.analyse_type_str)
            #structure = str(result.structure)
            #analyse_structure = str(result.analyse_structure)
            if result.search_result != None:
                ret_data["url"] = result.search_result.url.encode('utf-8','ignore')
                ret_data["docid"] = str(result.search_result.docid)
                ret_data["title"] = result.search_result.title.encode('utf-8','ignore')
                #abstract = result.search_result.abstract.encode('gbk','ignore')
            else:
                break
            ret_sql.append(ret_data)
            if int(result.rank) >= int(self.task_info["topn"]):
                break

        #信息入库
        sql = self.mk.build_mutil_sql(ret_sql)
        if len(sql) > 0:
            self.mk.db.query(sql)
            #self.sql_fp.write(sql+"\n")
            #self.sql_fp.flush()

        #解析完成的，添加到已完成列表里
        self.parsed_set.add(html_file)
        self.parsed_fp.write(html_file+"\n")
        self.parsed_fp.flush()
        return True

    ##########################################################################################
    #校验该任务是否可解析，以及一些必要的配置项是否存在
    ##########################################################################################
    def check_task(self):
        if not self.conf.has_key("MYSQL_MARK_OSS_HOST"):
            sys.stderr.write("can`t find mysql conf info \n")
            return False
        if not self.conf.has_key("SEED_FILE_DIR"):
            sys.stderr.write("can`t find seed file conf info \n")
            return False
        self.mk = mark_oss(user=self.conf["MYSQL_MARK_OSS_USER"],
                      passwd=self.conf["MYSQL_MARK_OSS_PASSWD"],
                      host=self.conf["MYSQL_MARK_OSS_HOST"],
                      db=self.conf["MYSQL_MARK_OSS_DB"],
                      port=int(self.conf["MYSQL_MARK_OSS_PORT"]),
                      charset=self.conf["MYSQL_MARK_OSS_CHARSET"]
        )
        self.task_info = self.mk.get_task_info(self.task_id)
        if len(self.task_info) <= 0:
            sys.stderr.write("fetch task info failed\n")
            return False
        if self.task_info["status"] in ["4","5"]:
            sys.stderr.write("this task is labeling or reviewing.....\n")
            return False
        return True

    ##########################################################################################
    #初始化一些配置信息项，如引擎信息、任务信息、种子文件位置
    ##########################################################################################
    def __init_conf__(self):
        #种子文件
        seed_file = self.conf["SEED_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".seed"
        utils.mk_sure_dir(seed_file)
        self.seed_fp = open(seed_file, "r")

        #解析完成的html文件
        parsed_log = self.conf["LOG_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".parsed"
        utils.mk_sure_dir(parsed_log)
        self.parsed_fp = open(parsed_log,"a+")
        for d in self.parsed_fp:
            self.parsed_set.add(d.strip())

        #日志文件
        log_file = self.conf["LOG_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".log"
        utils.mk_sure_dir(log_file)
        self.logger = logging.getLogger(os.path.basename(__file__))
        self.logger.setLevel(logging.DEBUG)
        formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
        file_handler = logging.handlers.TimedRotatingFileHandler(log_file, when="D", backupCount=7, encoding="gbk")
        file_handler.setFormatter(formatter)
        self.logger.addHandler(file_handler)
        self.wp_parse = WebPageParser(self.logger)


        sql_file = self.conf["TASK_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".sql"
        utils.mk_sure_dir(sql_file)
        self.sql_fp = open(sql_file,"a+")

        #读取引擎信息
        eid_str = self.task_info["engine"].split("\t")
        eid_str = ",".join(eid_str)
        self.engineList = self.mk.get_engine_list(eid_str)

    ##########################################################################################
    #获取解析时的配置文件位置，返回绝对路径
    ##########################################################################################
    def __get_srpp_config_file__(self, html_file):
        tmp_dir_name = os.path.dirname(html_file)
        tmp_base_name = os.path.basename(tmp_dir_name)
        if tmp_base_name.find("_") <= 0:
            return None
        eid = tmp_base_name[tmp_base_name.find("_")+1:]
        eid = int(eid)
        if eid <= 0:
            return None
        if eid not in self.engineList:
            return None
        einfo = self.engineList[eid]
        if not einfo.has_key("name"):
            return None
        ename = einfo["name"]
        if ename.find("sogou") >= 0:
            return self.conf["SRPP_CONFIG_SOGOU"]
        elif ename == "baidu":
            return self.conf["SRPP_CONFIG_BAIDU"]
        elif ename == "360":
            return self.conf["SRPP_CONFIG_360SO"]
        return None