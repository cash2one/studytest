#!/usr/local/bin/python
# coding=gbk
#######################################################
#name:      webpage_config.py
#author:    chillbu
#create:    20130408
#modify:    20130418 - 将规则定义移到parser_define.py
#
#desc: 定义download与parse模块的配置类
#
#######################################################  

import os
import re
import sys
import logging
import xml.etree.ElementTree as ET
from common.parser_define import PathRule,RangeRule,ItemRule,AD_TYPE_LIST

    
class WebPageConfig:
    '''页面分析器配置
    '''
    def __init__(self, logger = None):
        '''任务信息
            taskid, engine...
        '''
        self.encoding = ""
        self.taskid = "0"        
        self.engine = ""
        self.path = "../download"
        '''下载配置信息            
        '''
        self.method = "wget"        #下载方法: phantomjs/wget
        self.url_pattern = ""
        self.proxy = ""        
        self.ua = "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/525.13 (KHTML, like Gecko) Chrome/0.A.B.C Safari/525.13"
        self.sleep = 0          #下载后睡眠时间(秒)
        self.retry = 0          #下载重试次数
        '''分析器规则            
        '''
        self.container_rules = []
        self.item_rules = []
        self.redirect_url_scheme = ""
        self.redirect_url_scheme_re = None
        self.node_attrs = []
        '''页面处理规则            
        '''
        self.remove_rules = []
        
        if logger == None:
            self.logger = logging.getLogger(os.path.basename(__file__))
            self.logger.setLevel(logging.DEBUG)
            ch = logging.StreamHandler()
            ch.setLevel(logging.DEBUG)
            formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
            ch.setFormatter(formatter)
            self.logger.addHandler(ch)
        else:
            self.logger = logger
        
    def ParseString(self, config_string):
        '''NOTICE:
            python的xml解析模块只支持utf8编码的内容，所以需要先转换编码            
        '''
        conf_xml = ET.fromstring(config_string.decode("gbk", "ignore").encode("utf-8", "ignore"))
        if self.__ParseCommonConfig__(conf_xml) != 0:
            self.logger.error("Parse common config failed!")
            return -1
        if self.__ParseDownloaderConfig__(conf_xml) != 0:
            self.logger.error("Parse download config failed!")
            return -1
        if self.__ParseParserConfig__(conf_xml) != 0:
            self.logger.error("Parse parser config failed!")
            return -1
        if self.__ParsePageConfig__(conf_xml) != 0:
            self.logger.error("Parse page config failed!")
            return -1
        return 0

    def ParseFile(self, config_file):
        return self.ParseString(open(config_file).read())
               
    def ToString(self):
        string = "\n*********** List config info ***************\n" \
                 "taskid: %s\nengine: %s\nencoding: %s\npath: %s\nUA: %s\nredirect: %s\n" \
                 "method: %s\nurl_pattern: %s\nproxy: %s\nsleep: %s\nretry: %s\nnode_attr: %s\n" \
                 % (self.taskid, self.engine, self.encoding, self.path, self.ua, self.redirect_url_scheme, \
                    self.method, self.url_pattern, self.proxy, str(self.sleep), str(self.retry), str(self.node_attrs))
        for index,rule in enumerate(self.remove_rules):
            string += "\n----------[[Remove#%s]]----------\n" \
                      "path: %s\n" % (str(index+1), rule.GetPath())
        for index,rule in enumerate(self.container_rules):
            start_path = None
            end_path = None
            if rule.start_path_rule != None:
                start_path = rule.start_path_rule.GetPath()
            if rule.end_path_rule != None:
                end_path = rule.end_path_rule.GetPath()
            string += "\n----------[[Container#%s]]----------\n" \
                      "name: %s\nid: %s\ndesc: %s\nlevel: %s\nstart: %s\nend: %s\n" \
                      % (str(index), rule.name, str(rule.id), rule.desc, rule.level, start_path, end_path)
        for index,rule in enumerate(self.item_rules):
            string += "\n----------[[Item#%s]]----------\n" \
                      "name: %s\nid: %s\ndesc: %s\ncontainer_id: %s\nset_structure: %s\nparent_item_id: %s\n" \
                      "is_eval: %s\nis_keep: %s\nis_rank: %s\nis_save: %s\n" \
                      % (str(index+1), rule.name, str(rule.id), rule.desc, rule.set_structure, str(rule.container_id), \
                      str(rule.parent_item_id), str(rule.is_eval), str(rule.is_keep), str(rule.is_rank), str(rule.is_save))
            string += "cond_rules:\n"
            for cond_rule in rule.cond_rules:
                string += "\t%s: %s\n" % (cond_rule.name, cond_rule.GetPath())
            string += "get_rules:\n"
            for get_rule in rule.get_rules:
                string += "\t%s: %s\n" % (get_rule.name, get_rule.GetPath())
            string += "container_rule:\n"
            if rule.container_rule != None:
                start_path = None
                end_path = None
                if rule.container_rule.start_path_rule != None:
                    start_path = rule.container_rule.start_path_rule.GetPath()
                if rule.container_rule.end_path_rule != None:
                    end_path = rule.container_rule.end_path_rule.GetPath()
                string += "\tid: %s\n\tdesc: %s\n\tlevel: %s\n\tstart: %s\n\tend: %s\n" \
                          % (str(rule.container_rule.id), rule.container_rule.desc, rule.container_rule.level, start_path, end_path)
                          
        return string
        
        
    '''Private functions       
    '''     
    def __ParseCommonConfig__(self, conf_xml):
        #encoding
        found = conf_xml.findall("./Config/Encoding")
        if len(found) == 0:
            self.logger.error("/Config/Encoding not found!")
            return -1
        self.encoding = found[0].text        
        #taskid
        found = conf_xml.findall("./Config/Taskid")
        if len(found) == 0:
            self.logger.warning("/Config/Taskid not found!")
        else:
            self.taskid = found[0].text
        #engine
        found = conf_xml.findall("./Config/Engine")
        if len(found) == 0:
            self.logger.warning("No encoding found! path=/Config/Engine")
        else:
            self.engine = found[0].text
        #path
        found = conf_xml.findall("./Config/Path")
        if len(found) != 0 and found[0].text != None:
            self.path = found[0].text       
        
        return 0
    
    def __ParseDownloaderConfig__(self, conf_xml):
        found = conf_xml.findall("./Download/UrlPattern")
        if len(found) == 0:
            self.logger.error("/Download/UrlPattern not found!")
            return -1
        self.url_pattern = found[0].text
        
        found = conf_xml.findall("./Download/Method")
        if len(found) != 0 and found[0].text != None:
            self.method = found[0].text
        
        found = conf_xml.findall("./Download/Sleep")
        if len(found) != 0 and found[0].text != None:
            self.sleep = found[0].text                    
            
        found = conf_xml.findall("./Download/UA")
        if len(found) != 0 and found[0].text != None:
            self.ua = found[0].text
        
        found = conf_xml.findall("./Download/Retry")
        if len(found) != 0:
            self.retry = found[0].text
            
        found = conf_xml.findall("./Download/Proxy")
        if len(found) != 0:
            self.proxy = found[0].text
            
        return 0
            
    def __ParseParserConfig__(self, conf_xml):
        #redirect url
        found = conf_xml.findall("./Parse/Redirect")
        if len(found) != 0:
            if found[0].text.lower() == "true":
                self.redirect_url_scheme = found[0].attrib["scheme"]
                self.redirect_url_scheme_re = re.compile(self.redirect_url_scheme, re.S)        
        #exclude node attribute
        found = conf_xml.findall("./Parse/NodeAttr")
        if len(found) != 0:
            if len(found[0].text) > 0:
                self.node_attrs = found[0].text.strip().split(",")
        #container element
        found = conf_xml.findall("./Parse/Container")
        if len(found) == 0:
            self.logger.error("/Parse/Container not found!")
            return -1
        for container in found:
            rule = self.__ParseContainerElement__(container)
            self.container_rules.append(rule)
        #item element
        found = conf_xml.findall("./Parse/Item")
        for item in found:
            rule = self.__ParseItemElement__(item)
            self.item_rules.append(rule)
            
        return 0
        
    def __ParsePageConfig__(self, conf_xml):
        #remove rule
        found = conf_xml.findall("./Page/Remove")
        for item in found:
            rule = self.__ParsePathElement__(item)
            self.remove_rules.append(rule)
            
        return 0
    
    def __ParseContainerElement__(self, conf_xml):
        rule = RangeRule()
        rule.name = conf_xml.tag
        if "level" not in conf_xml.attrib:
            self.logger.error("No level attribute of container!")
            return -1
        else:
            rule.level = conf_xml.attrib["level"]
        if "id" in conf_xml.attrib:
            rule.id = conf_xml.attrib["id"]
        if "desc" in conf_xml.attrib:
            rule.desc = conf_xml.attrib["desc"].encode('gbk','ignore')
        if "method" in conf_xml.attrib:
            rule.method = conf_xml.attrib["method"].encode('gbk','ignore')
        #start
        found = conf_xml.findall("./Start")
        if len(found) == 0:
            self.logger.warning("No start path found!")
        else:
            rule.start_path_rule = self.__ParsePathElement__(found[0])
        #end
        found = conf_xml.findall("./End")
        if len(found) == 0:
            self.logger.warning("No end path found!")
        else:
            rule.end_path_rule = self.__ParsePathElement__(found[0])
        
        return rule
        
    def __ParseItemElement__(self, conf_xml):
        rule = ItemRule()
        rule.name = conf_xml.tag
        if "id" in conf_xml.attrib:
            rule.id = conf_xml.attrib["id"]
        if "desc" in conf_xml.attrib:
            rule.desc = conf_xml.attrib["desc"].encode('gbk','ignore')
        if "parent" in conf_xml.attrib:
            rule.parent_item_id = conf_xml.attrib["parent"]
        if "container" in conf_xml.attrib:
            rule.container_id = conf_xml.attrib["container"]
        if "eval" in conf_xml.attrib:
            if conf_xml.attrib["eval"].lower() == "false":
                rule.is_eval = False
        if "keep" in conf_xml.attrib:
            if conf_xml.attrib["keep"].lower() == "false":
                rule.is_keep = False
        if "rank" in conf_xml.attrib:
            if conf_xml.attrib["rank"].lower() == "false":
                rule.is_rank = False
        if "save" in conf_xml.attrib:
            if conf_xml.attrib["save"].lower() == "false":
                rule.is_save = False
        if "auto_parse" in conf_xml.attrib:
            if conf_xml.attrib["auto_parse"].lower() == "true":
                rule.is_auto_parse = True
        if "ad_type" in conf_xml.attrib:
            ad_type = conf_xml.attrib["ad_type"]
            if ad_type != None and ad_type.isdigit() and int(ad_type) in AD_TYPE_LIST:
                rule.ad_type = int(ad_type)
        #node
        found = conf_xml.findall("./Node")
        if len(found) != 0:
            for node in found:
                rule.cond_rules.append(self.__ParsePathElement__(node))
        #structure
        found = conf_xml.findall("./Struct")
        if len(found) != 0:
            for node in found:
                rule.cond_rules.append(self.__ParsePathElement__(node))
        #path
        found = conf_xml.findall("./Path")
        if len(found) != 0:
            for node in found:
                rule.cond_rules.append(self.__ParsePathElement__(node))
        #remove
        found = conf_xml.findall("./Remove")
        if len(found) != 0:
            for node in found:
                rule.get_rules.append(self.__ParsePathElement__(node))
        #url
        found = conf_xml.findall("./Url")
        if len(found) != 0:
            for node in found:
                rule.get_rules.append(self.__ParsePathElement__(node))
        #title
        found = conf_xml.findall("./Title")
        if len(found) != 0:
            for node in found:
                rule.get_rules.append(self.__ParsePathElement__(node))
        #abstract
        found = conf_xml.findall("./Abstract")
        if len(found) != 0:
            for node in found:
                rule.get_rules.append(self.__ParsePathElement__(node))
        #container
        found = conf_xml.findall("./Container")
        if len(found) != 0:
            rule.container_rule = self.__ParseContainerElement__(found[0])
        #set structure
        found = conf_xml.findall("./SetStructure")
        if len(found) != 0:
            rule.set_structure = found[0].text
        
        return rule
        
    def __ParsePathElement__(self, conf_xml):        
        rule = PathRule()
        rule.name = conf_xml.tag
        if rule.name == "Start" or rule.name == "End" or rule.name == "Path" or rule.name == "Remove":            
            rule.path = conf_xml.text
        elif rule.name == "Node":
            rule.node_name = conf_xml.text
        elif rule.name == "Struct":
            rule.structure = conf_xml.text
            rule.structure_re = re.compile(rule.structure.replace("|","\|"), re.S)
        elif rule.name == "Url":
            #NOTICE(chillbu): remove Url rule path last 'a' node
            nodes = conf_xml.text.strip().split('@')
            if nodes[-1] != 'a':
                self.logger.warning("Url rule's last node not a!")
            rule.path = '@'.join(nodes[0:-1])
            rule.gets.append("url")            
        elif rule.name == "Abstract":
            rule.path = conf_xml.text
            rule.gets.append("abstract")
        elif rule.name == "Title":
            rule.path = conf_xml.text
            rule.gets.append("title")
        else:
            self.logger.error("Unknown path rule[%s]" % conf_xml.tag)
            return None
        
        for name,value in conf_xml.attrib.items():
            if name.lower() == "text":
                rule.text = value.encode('gbk','ignore')
                rule.text_re = re.compile(value.encode('gbk','ignore'), re.S)
            else:
                rule.node_attr[name] = value
                if value != None and value != "":               
                    rule.node_attr_re[name] = re.compile(value, re.S)
                else:
                    rule.node_attr_re[name] = None
        return rule
        

if __name__ == "__main__":
    args = sys.argv
    if (len(args) < 2):
        print "USAGE: %s <conf_file>" %(args[0])
        sys.exit()
        
    conf_file = args[1]
    
    log_name = "webpage_config.log"
    logger = logging.getLogger(os.path.basename(__file__))
    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
    file_handler = logging.FileHandler(log_name, "w")
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    
    conf_parser = WebPageConfig(logger)
    conf_parser.ParseFile(conf_file)
    logger.info(conf_parser.ToString())
    
