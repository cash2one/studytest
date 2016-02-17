#!/usr/local/bin/python
# -*- coding: utf-8 -*-
#######################################################
# Copyright (c) 2012, Tencent Inc.
# All rights reserved
#
# Author: Manzhao BU <chillbu@tencent.com>
# Create: 20130410
#
# Desc: 根据自定义规则，分析搜索结果页
#
#######################################################

import re
import os
import sys
import logging
from bs4 import BeautifulSoup,NavigableString,Tag
from webpage_config import WebPageConfig
from common.base_parser import BaseParser
from common.base_parser import StructureParser
from common.base_parser import IDParser
import common.eva_common as common
import common.parser_define as webpage
import traceback
reload(sys)
sys.setdefaultencoding('utf-8')

class WebPageParser:
    def __init__(self, logger = None):
        self.config = None
        self.parser = None
        self.struct_parser = None
        self.id_parser = None
        self.display_none_re = re.compile(u"display\\s*:\\s*none", re.S|re.I)
        self.result_anchor_string = "<a id=\"rank%s\" name=\"element%s\" SOSO_EVA_ANCHOR />"
        self.result_anchor_tail_string = "<a id=\"rank%s_tail\" name=\"element%s_tail\"/>"
        '''页面信息结构
        '''
        self.page_info = webpage.PageInfo()
        self.page_element_list = []
        self.page_element_id = 0
        self.container = []
        
        self.result_num = 0
        self.child_result_num = 0
        self.url_result_num = 0
        
        self.current_iter_index = 0
        
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
        
    '''Public interface        
    '''
    def Init(self, config):        
        if isinstance(config, WebPageConfig):
            self.config = config
        elif isinstance(config, str):            
            self.config = WebPageConfig(self.logger)
            if self.config.ParseFile(config) != 0:
                self.logger.error("Parse config file failed! file="+config)
                return 1
        else:
            self.logger.error("Not config path or object!")
            return 1
        
        #self.logger.debug("################# Config ##################")
        #self.logger.debug(self.config.ToString())
        
        self.parser = BaseParser(self.logger)
        self.struct_parser = StructureParser(self.logger)
        self.id_parser = IDParser(self.logger)
        
        return 0  
    
    def Reset(self):
        self.page_info = webpage.PageInfo()
        self.page_element_list = []
        self.page_element_id = 0
        self.result_num = 0
        self.child_result_num = 0
        self.url_result_num = 0
        self.current_iter_index = 0
        
    def ParseHTML(self, html_string):
        self.logger.info("********************* Reset begin ******************")
        self.parser.Reset()        
        self.logger.info("********************* Reset end ******************")
        self.logger.info("********************* Parse HTML begin ***********************")
        self.logger.info(self.config.encoding)
        self.parser.Parse(html_string, self.config.encoding)
        self.logger.info("Parse html string. html_len="+str(len(html_string)))        
        #抽取页面元素
        for container_rule in self.config.container_rules:
            ret = self.__HandleContainerRule__(container_rule)
            if len(ret) == 0:
                continue
            self.page_element_list += ret
        self.__ParseElementList__()
        self.logger.info("********************* Parse HTML end ***********************")
        #统计抽取结果信息
        self.__StatResult__()
        
    def ParseResultHTML(self, html_string):
        self.logger.info("********************* Parse Result HTML begin ***********************")
        self.parser.Reset()
        soup = BeautifulSoup(html_string, "lxml",from_encoding="utf-8")
        element = webpage.PageElement()
        element.bs_tag = soup.body.next
        self.page_element_list.append(element)
        self.__ParseElementList__()
        self.logger.info("********************* Parse Result HTML end ***********************")
        #统计抽取结果信息
        self.__StatResult__()
            
    def __ParseElementList__(self):
        #遍历页面元素
        self.logger.info("Loop to parse element list:")
        self.current_iter_index = 0        
        for element in self.page_element_list:
            self.logger.info("Item#%d: " % element.index)
            self.current_iter_index += 1
            if element.item_id in webpage.NOT_DISPLAY_ITEMS:
                self.logger.info("Skip for not display element[%d]" % element.item_id)
                continue
            #基本信息抽取
            element.node_info = self.parser.GetNodeInfo(element.bs_tag, self.config.node_attrs)
            element.path = self.parser.GetNodeSelector(element.bs_tag)
            element.analyse_path = element.path
            element.structure = self.struct_parser.Parse(element.bs_tag, self.config.engine)
            element.analyse_structure = element.structure
            #规则抽取
            is_hit_item_rule = False
            for item_rule in self.config.item_rules:
                if self.__HandleItemRule__(item_rule, element) == True:
                    is_hit_item_rule = True
                    break
            #
            # open the tag not save
            #
            if element.is_save == False:
                self.logger.info("Skip for not save element")
                continue
            #
            # open the tag not keep
            #
            if element.is_keep == False:
                self.logger.info("Skip for not keep element")
                continue

            if is_hit_item_rule == False:
                self.logger.info("Auto parse element")
                self.__SearchResultAutoParse__(element)
            print "----------------------print log "
            self.__GetUrl__(self.config.engine, element)
            
            if element.search_result.url == "javascript:;" or element.search_result.url == "about:blank" or element.search_result.url == "javascript:void(0)":
                element.search_result.url = ""
            
            #抽取ID信息（依赖于URL抽取）
            [type_str, ana_type_str] = self.id_parser.Parse(self.config.engine, element.url_tag, element.bs_tag)
            if type_str != None and ana_type_str != None:
                element.type_str = type_str
                element.analyse_type_str = ana_type_str
        #再次遍历结果：清理并计算rank
        self.logger.info("Loop result again: Clean up not display & Ranking")
        clean_list = []
        rank = 0
        for element in self.page_element_list:
            if element.item_id in webpage.NOT_DISPLAY_ITEMS:
                self.logger.info("Remove not display element#%d[%d]" % (element.index, element.item_id))
                continue
            if element.is_rank == False:
                continue
            if len(element.search_result.raw_url) <= 0:
                continue
            rank += 1
            element.rank = rank
            #去掉title里的空白字符
            title = element.search_result.title.replace("\n","")
            title = title.replace("\t","")
            title = title.replace(" ","")
            element.search_result.title = title
            clean_list.append(element)
            #统计广告的数量
            if element.ad_type == webpage.AD_TYPE_PZL:
                self.page_info.ad_pzl_num += 1
            elif element.ad_type == webpage.AD_TYPE_TOP:
                self.page_info.ad_top_num += 1
            elif element.ad_type == webpage.AD_TYPE_BOTTOM:
                self.page_info.ad_bottom_num += 1
        self.page_element_list = clean_list                                                
    
    '''获取重定向URL
    '''
    def RedirectUrl(self):
        if self.config.redirect_url_scheme == "":
            return
            
        redirect_urls = {}        
        self.__LoopToRedirectUrl__(self.page_element_list, redirect_urls)
        
    def __LoopToRedirectUrl__(self, result_list, redirect_urls):
        for element in result_list:
            if len(element.children) > 0:
                self.__LoopToRedirectUrl__(element.children, redirect_urls)
            if element.search_result == None:
                continue            
            if len(element.search_result.url) == 0:
                continue
            url = element.search_result.url
            if url in redirect_urls:                
                element.search_result.url = common.NormalizeUrl(redirect_urls[url], self.config.url_pattern)
                element.search_result.docid = int(common.GetDocID(element.search_result.url), 10)
            else:                
                if not self.config.redirect_url_scheme_re.match(url):                                               
                        continue  #不需要重定向
                else:
                    self.logger.info("GetRedirect: " + url)
                    redirect_url = common.GetRedirectURL(url)
                    if redirect_url != "":
                        element.search_result.url = common.NormalizeUrl(redirect_url, self.config.url_pattern)
                        redirect_urls[url] = element.search_result.url
                        element.search_result.docid = common.GetDocID(element.search_result.url)                                            
                        self.logger.info("DocID=" + str(element.search_result.docid))                    
        
    '''页面处理
    '''
    def ProcessPage(self):
        element = PageElement()
        element.bs_tag = self.parser.soup
        for rule in self.config.remove_rules:            
            if self.__HandlePathRule__(rule, element) == True:
                self.logger.debug("Remove path: "+rule.path)
        return
    
    def RebuildAnchorPage(self):
        for result in self.page_element_list:        
            if result.item_id in webpage.NOT_DISPLAY_ITEMS:
                continue
            head_string = self.result_anchor_string % (str(result.rank), str(result.index))
            soup = BeautifulSoup(head_string, "lxml",from_encoding="utf-8")
            result.bs_tag.insert_before(soup.a)
            tail_string = self.result_anchor_tail_string % (str(result.rank), str(result.index))
            soup = BeautifulSoup(tail_string, "lxml",from_encoding="utf-8")
            result.bs_tag.insert_after(soup.a)
        return self.parser.soup.encode(formatter="html")
    
    def PrintResultList(self):
        self.logger.info("List result info:")
        for result in self.page_element_list:
            self.logger.info(result.ToString())
    
    '''Private internal functions
    '''
    def __StatResult__(self):
        #统计抽取结果信息
        self.result_num = len(self.page_element_list)
        self.child_result_num = 0
        self.url_result_num = 0
        for element in self.page_element_list:
            if element.search_result != None and len(element.search_result.raw_url) > 0:
                self.url_result_num += 1
            if element.parent_index != 0:
                self.child_result_num += 1       
        self.logger.info("Result stat:\n\tcontainer: %d\n\tresults: %d\n\tchild_results: %d\n\tno_url_result: %d\n" \
                            % (self.page_info.container_num, self.result_num, self.child_result_num, self.result_num-self.url_result_num))
        
    def __HandleItemRule__(self, rule, element):
        #检查item的container_id是否匹配
        if rule.container_id != 0 and rule.container_id != element.container_id:
            return False
        #检查item的parent_item_id是否匹配
        if rule.parent_item_id != 0 and rule.parent_item_id != element.parent_item_id:
            return False
        #检查item与cond_rules是否匹配
        for cond_rule in rule.cond_rules:
            if self.__HandlePathRule__(cond_rule, element) == False:
                return False        
        self.logger.info("Match item rule#%s: %s" % (rule.id, rule.desc))
        element.item_id = rule.id
        element.item_desc = rule.desc
        element.is_eval = rule.is_eval
        element.is_keep = rule.is_keep
        element.is_rank = rule.is_rank
        element.is_save = rule.is_save
        element.ad_type = rule.ad_type
        if rule.set_structure != None:
            element.analyse_structure = rule.set_structure
        
        #抽取内容
        element.search_result = webpage.SearchResult()
        for get_rule in rule.get_rules:
            self.__HandlePathRule__(get_rule, element)
        if rule.is_auto_parse == True:
            self.logger.info("Auto parse element")
            self.__SearchResultAutoParse__(element)
            
        #抽取子结果
        if rule.container_rule != None:
            #将container元素加入元素列表
            for found_element in reversed(self.__HandleContainerRule__(rule.container_rule, element)):
                #self.page_element_list.insert(element.index, found_element)
                self.page_element_list.insert(self.current_iter_index, found_element)
            #self.page_element_list += self.__HandleContainerRule__(rule.container_rule, element)
        
        return True
        
    def __HandleContainerRule__(self, rule, parent_element = None):
        container_list = []
        '''获取container下的所有子节点
            返回结果结构:
            container list: [ container1, container2, ..., containerN ]
            其中, containeri = [ result_soup1, ..., result_soupN ]
        '''
        is_top_level = True
        root_soup = None
        if parent_element != None:
            is_top_level = False         #是否解析顶层结果
            root_soup = parent_element.bs_tag   #设置开始查找的soup对象
        
        start = None
        start_attr = None
        end = None
        end_attr = None
        if rule.start_path_rule != None:
            start = rule.start_path_rule.path
            start_attr = rule.start_path_rule.node_attr_re
        if rule.end_path_rule != None:
            end = rule.end_path_rule.path
            end_attr = rule.end_path_rule.node_attr_re

        #对container的定义，允许使用bs自己的selector选取，此时start和end的attr将被忽略
        if rule.method == "selector":
            container_list = self.parser.RangeParseBySelector(start, end, rule.level, root_soup)
        else:
            container_list = self.parser.RangeParse(start, start_attr, end, end_attr, rule.level, root_soup)
    
        #记录页面container的数目
        if len(container_list) == 0:
            self.logger.info("No container found")
            return []
        if is_top_level == True:            
            self.page_info.container_num = len(container_list)
            self.container += self.parser.FindNode(start, start_attr)
        self.logger.info("Found %d container of rule: %s" \
                            % (self.page_info.container_num, str(rule.id)))        
        
        #遍历container
        result_list = []
        for container_index,container in enumerate(container_list):
            self.logger.info("Begin parse #%d container %d items:" % (container_index, len(container)))
            #遍历container的元素
            for element_index,element_node in enumerate(container):
                if element_node.name.lower() == "style":  #判断element是否js或者css标签
                    continue
                if element_node.name.lower() == "script":
                    continue
                if element_node.name.lower() == "br":
                    continue
                if element_node.name.lower() == "link":
                    continue
                if "style" in element_node.attrs and self.display_none_re.match(element_node.attrs["style"]) and self.config.engine != "sogou":
                    continue
                if len(list(element_node.stripped_strings)) == 0 and len(element_node.findAll("iframe")) == 0:
                    continue
                element = webpage.PageElement()
                #分配ID
                self.page_element_id += 1
                element.index = self.page_element_id
                if is_top_level == True:
                    self.logger.info("Element[%d]-Index[%d]: %s" % (element_index, element.index, element_node.name))
                else:
                    element.parent_index = parent_element.index
                    element.parent_item_id = parent_element.item_id
                    self.logger.info("Element[%d]-Index[%d-%d]: %s" % (element_index, element.parent_index, element.index, element_node.name))
                                
                element.bs_tag = element_node
                element.container_id = rule.id
                result_list.append(element)
                
        return result_list                        
    def __GetUrl__(self, engine, element):
        #__GetUrl__ by liulanbo
        if engine == "sogou":
            ignore_urlclass = ["notiHelp","openCfg"]
            ignore_h3class = ["setTitle2"]
            element.search_result.url = "" 
            #print "__GetUrl__ element desc=======:%s %s" % (element.node_info, self.display_none_re)
            if "style" in element.bs_tag.attrs and self.display_none_re.match(element.bs_tag.attrs["style"]):
                #print "display none true===="
                return False
            #抽取指定URL SOGOU 取H3里的A标签
            #print "__GetUrl__ element:%s" % (element.bs_tag)
            for url_node in element.bs_tag.find_all("a"):
                if url_node == None:
                    continue
                if url_node.parent.name.lower() != "h3":
                    continue
                if "class" in url_node.parent.attrs and "".join(url_node.parent.attrs["class"]) in ignore_h3class:
                    continue
                if "href" not in url_node.attrs:
                    continue
                if "class" in url_node.attrs and "".join(url_node.attrs["class"]) in ignore_urlclass:
                    continue
                element.url_tag = url_node
                element.search_result.raw_url = url_node["href"].strip()
                element.search_result.url = common.NormalizeUrl(element.search_result.raw_url, self.config.url_pattern)		    
                element.search_result.docid = int(common.GetDocID(element.search_result.url), 10)
                element.search_result.title = url_node.text.strip()
                break

            if element.search_result.raw_url and (element.search_result.raw_url.encode().find("wenwen.soso.com/z/Search") != -1 or element.search_result.raw_url.encode().find("zhishi.sogou.com/zhishi") != -1):
                multi_url = ""
                url_tap = ""
                url_num = 0
                for url_node in element.bs_tag.find_all("a"):
                    if url_node == None:
                        continue
                    if url_node.parent.name.lower() == "h3":
                        continue
                    if multi_url != "":
                        url_tap = "|"
                    url_num = url_num + 1
                    multi_url += url_tap + url_node["href"].strip()
                    if url_num == 2:
                        break
                if multi_url != "":
                    element.search_result.raw_url = multi_url
                    element.search_result.url = common.NormalizeUrl(multi_url, self.config.url_pattern)
                
            #print "__GetUrl__#element.url_tag%s" % (url_node)
            return True
        else:
            return False
    def __HandlePathRule__(self, rule, element):
        #定位路径
        found_nodes = [element.bs_tag]
        if rule.path != None and len(rule.path) > 0:
            found_nodes = self.parser.FindNode(rule.path, rule.node_attr_re, element.bs_tag)                
            if found_nodes == None or len(found_nodes) == 0:
                return False
            
        for found_node in found_nodes:
            #判断是否匹配属性
            if rule.node_name != None and len(rule.node_name) > 0 \
                and self.parser.MatchNode(found_node, rule.node_name, rule.node_attr_re) == False:
                continue
            #判断文本是否匹配
            if rule.text_re != None and not rule.text_re.match(found_node.text.strip()):
                continue
            #判断结构是否匹配
            if rule.structure_re != None and not rule.structure_re.match(element.analyse_structure):
                continue
            #优先处理Remove规则
            if rule.name.lower() == "remove":
                found_node.extract()
                continue
            #配置抽取属性，则进行处理
            for attr_name in rule.gets:
                if attr_name.lower() == "url":
                    url_node = self.parser.FindFirstChildNode(found_node, "a")
                    if url_node == None:
                        continue
                    if "href" not in url_node.attrs:
                        self.logger.error("Attr[href] not found")
                        continue
                    element.url_tag = url_node
                    element.search_result.raw_url = url_node["href"].strip()
                    element.search_result.url = common.NormalizeUrl(element.search_result.raw_url, self.config.url_pattern)		    
                    element.search_result.docid = int(common.GetDocID(element.search_result.url), 10)
                    element.search_result.title = url_node.text.strip()
                elif attr_name.lower() == "abstract":
                    element.search_result.abstract = found_node.text.strip() 
                elif attr_name.lower() == "title":
                    element.search_result.title = found_node.text.strip()                
                else:
                    self.logger.error("Unknown Attr[%s]" % attr_name)
            #满足所有条件，找到了节点
            return True
        return False
    
    #试验:
    #1. 取第一个a节点作为url
    #2. 若无a节点或a节点在文字节点之后，取第一个文字节点作为标题
    #3. 取整个节点的(非script, style)文字作为摘要，若有标题则删除标题文字
    def __SearchResultAutoParse__(self, element):
        found = False
        url = ""
        url_tag = ""
        title = ""
        abstract = ""
        #Find url and title
        for url_node in element.bs_tag.find_all("a"):
            if len(url_node.text.strip()) == 0:
                continue
            if "href" in url_node.attrs:
                if url_node["href"] == "#": #Not url node
                    continue
                url = url_node["href"].strip()
                #Get title: soso url node has script or style, need to skip it
                for child in url_node.descendants:
                    if isinstance(child, NavigableString):
                        if child.parent.name.lower() in webpage.NOT_ANCHOR_ITEMS_NAME:
                            continue
                        #NOTE@chillbu: To avoid abs in a node like wap result
                        #if child.parent.name.lower() not in ["em","span","h3"]:
                        #    break
                        title += child.string.strip()
                url_tag = url_node
                found = True
                break
            else:
                self.logger.error("Attr[href] not found")
                break
        #Find abstract
        for child in element.bs_tag.descendants:
            if isinstance(child, NavigableString):
                if child.parent.name.lower() in webpage.NOT_ANCHOR_ITEMS_NAME:
                    continue
                if len(child.string.strip()) == 0:
                    continue
                if found == False:
                    title = child.parent.text.strip()
                    found = True
                abstract += child.string.strip()
            #特殊处理sogou里的微信公众账号等
            elif isinstance(child, Tag) and self.config.engine == "sogou":
                if child.name == "div" and \
                        "id" in child.attrs and \
                        child.attrs["id"].startswith("wx_title") and \
                        url.startswith("http://weixin.sogou.com"):
                    title = child.string.strip() + title

        if element.search_result == None:
            element.search_result = webpage.SearchResult()
        if url != "":
            element.url_tag = url_node
            element.search_result.raw_url = url
            element.search_result.url = common.NormalizeUrl(element.search_result.raw_url, self.config.url_pattern)		    
            element.search_result.docid = int(common.GetDocID(element.search_result.url), 10)
        element.search_result.title = title
        if abstract[0:len(title)] == title: #delete title if it's the first string of abs
            element.search_result.abstract = abstract[len(title):]
        else:
            element.search_result.abstract = abstract
        #for debug
        if len(element.search_result.raw_url) == 0:
            self.logger.debug("[%s] url not found!" % str(element.index))
        if len(element.search_result.title) == 0:
            self.logger.debug("[%s] title not found!" % str(element.index))
        if len(element.search_result.abstract) == 0:
            self.logger.debug("[%s] abs not found!" % str(element.index))
        if len(element.search_result.abstract) > 1024:
            self.logger.debug("[%s] abs too large! %d" % (str(element.index), len(element.search_result.abstract)))
            
        return
   
'''Main function
    Enterance of the script
'''    
if __name__ == "__main__":
    args = sys.argv
    if (len(args) < 3):
        print "USAGE: %s <conf_file> <html_file>" %(args[0])
        sys.exit()
    
    conf_file = args[1]
    html_file = args[2]   
    
    log_name = "parser.log"
    logger = logging.getLogger(os.path.basename(__file__))
    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter('[%(asctime)s %(filename)s:%(lineno)d] [%(levelname)s] %(message)s')
    file_handler = logging.FileHandler(log_name, "w")
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    
    config = WebPageConfig(logger)
    if config.ParseFile(conf_file) != 0:
        logger.error("Parse config file failed! file="+conf_file)
        sys.exit() 
        
    parser = WebPageParser(logger)
    if parser.Init(config) != 0:
        logger.error("Init WebPageParser failed!")
        sys.exit()
    
    parser.page_info.file_path = html_file
    parser.ParseHTML(open(html_file).read())
    
    parser.PrintResultList()
    logger.info(parser.RebuildAnchorPage())
    print "ad_pzl_num\t%d" % parser.page_info.ad_pzl_num
    print "ad_top_num\t%d" % parser.page_info.ad_top_num
    print "ad_bottom_num\t%d" % parser.page_info.ad_bottom_num
    for index,result in enumerate(parser.page_element_list):
        if result.search_result == None:
            continue
        print "\n\n-----------------------------------------"
        print "rank\t",result.rank
        print "url\t",result.search_result.url
        print "raw_url\t",result.search_result.raw_url
        print "title\t",result.search_result.title
        print "ad_type\t",result.ad_type
    
