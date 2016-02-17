#!/usr/bin/python
# coding=gbk

####################################################
# @author Liu Yongshuai<liuyongshuai@sogou-inc.com>
# ����ҳ��ʱ�Ĺ����࣬�����õ�
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
        self.task_id = task_id #�����ţ�Ŀǰ������
        self.conf = conf_info #���������������ļ�����Ϣ
        self.engineList = dict() #���õ����������ϸ��Ϣ
        self.task_info = dict() #�������ϸ��Ϣ
        self.seed_fp = "" #�����ļ�
        self.parsed_fp= "" #�ѽ�����ɵ�html�ļ��������ظ�����
        self.mk = None #����mark_oss�����ʵ��
        self.parsed_set = set() #�ѽ�������html�ļ���
        self.logger = "" #��������е���־�ļ�
        self.wp_parse = None #webpage_parser
        self.wp_confs = dict() #webpage_config
        self.sql_fp = "" #��ʱ�洢SQL����


    def run(self):
        #����һ������
        task_file = self.conf["TASK_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".task"
        os.system("/work/test_test/page_download/server/task %s" % task_file)

        #��ʼ������
        self.__init_conf__()
        #�Ƿ������
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
    #����һ��html�ļ����߼�������������⡢��ȡ�����ļ���
    ##########################################################################################
    def __parse_html__(self, query, html_file):
        if not os.path.isfile(html_file):
            return False
        if html_file in self.parsed_set:
            return True
        #��ȡ������
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

        #��html�ļ����򵥴���
        fp = open(html_file,"r")
        html_content = fp.read()
        fp.close()
        #�ļ���С�����⣬���ǰٶȵ���Ҫ������֤���ҳ��
        if len(html_content) <= 0 or (utils.is_baidu_verifycode(html_content) and \
                        int(eid) in self.engineList and \
                        self.engineList[int(eid)]["name"].find("baidu")>=0):
            os.remove(html_file)
            return False
        html_content = utils.strip_tags(html_content)

        #��ʼ����
        self.wp_parse.Init(config)
        self.wp_parse.Reset()
        self.wp_parse.ParseHTML(html_content)
        self.wp_parse.RedirectUrl()

        #���������
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

        #��Ϣ���
        sql = self.mk.build_mutil_sql(ret_sql)
        if len(sql) > 0:
            self.mk.db.query(sql)
            #self.sql_fp.write(sql+"\n")
            #self.sql_fp.flush()

        #������ɵģ���ӵ�������б���
        self.parsed_set.add(html_file)
        self.parsed_fp.write(html_file+"\n")
        self.parsed_fp.flush()
        return True

    ##########################################################################################
    #У��������Ƿ�ɽ������Լ�һЩ��Ҫ���������Ƿ����
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
    #��ʼ��һЩ������Ϣ���������Ϣ��������Ϣ�������ļ�λ��
    ##########################################################################################
    def __init_conf__(self):
        #�����ļ�
        seed_file = self.conf["SEED_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".seed"
        utils.mk_sure_dir(seed_file)
        self.seed_fp = open(seed_file, "r")

        #������ɵ�html�ļ�
        parsed_log = self.conf["LOG_FILE_DIR"].rstrip("/") + "/" + str(self.task_id) + ".parsed"
        utils.mk_sure_dir(parsed_log)
        self.parsed_fp = open(parsed_log,"a+")
        for d in self.parsed_fp:
            self.parsed_set.add(d.strip())

        #��־�ļ�
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

        #��ȡ������Ϣ
        eid_str = self.task_info["engine"].split("\t")
        eid_str = ",".join(eid_str)
        self.engineList = self.mk.get_engine_list(eid_str)

    ##########################################################################################
    #��ȡ����ʱ�������ļ�λ�ã����ؾ���·��
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