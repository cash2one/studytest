#!/usr/local/bin/python
# -*- coding: utf-8 -*-
#######################################################
#name:      base_parser.py
#author:    chillbu
#create:    20130410
#
#desc: 基于bs分析搜索结果页，对外提供两种分析接口
#       FindNode - 根据css selector查找tag
#       FindNodeByPath - 根据自定义path查找tag
#       RangeParse - 根据自定义path进行范围查找
#       StructureParse - 分析给定节点的结构
#
#######################################################

import sys
import re
import os
import types
import logging
from bs4 import BeautifulSoup,Comment,Tag
reload(sys)
sys.setdefaultencoding('utf-8')

class BaseParser:
    def __init__(self, logger = None):   
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
        self.soup = None        
        
    '''Public interface        
    '''
    def Parse(self, html_string, encoding):
        self.logger.info("-----------------in Parse")
        self.soup = BeautifulSoup(html_string,"lxml")
        self.logger.info("-----------------in Parse")
        #find out encoding
        if self.soup.declared_html_encoding is None or len(self.soup.declared_html_encoding) <= 0:
            found_encoding = encoding
            self.logger.info("declared encoding: "+ encoding)
        else:
            found_encoding = self.soup.declared_html_encoding
            self.logger.info("declared encoding: "+found_encoding)
        #re-encoding html
        new_html_string = html_string.decode(found_encoding, 'ignore').encode(encoding, 'ignore')
        self.logger.info("convert encoding to " + encoding)
        self.soup = BeautifulSoup(new_html_string, "lxml", from_encoding=encoding)
        #remove comments
        comments = self.soup.findAll(text=lambda text:isinstance(text, Comment))
        for comment in comments:
            comment.extract()            
        return self.soup
    
    def Reset(self):
        self.soup = None
    
    '''Find soup using self define path
        bs not support xpath query now, using self implementation
        path: html@body@div
        attrs: attr for last node
    '''               
    def FindNode(self, path, attrs, soup = None):                            
        found_soup_list = []
        nodes = None
        node_name = self.__GetLastNodeName__(path)   
        if soup == None:
            nodes = self.soup.findAll(name=node_name, attrs=attrs)
        else:
            nodes = soup.findAll(name=node_name, attrs=attrs)
            
        if len(nodes) <= 0:
            self.logger.debug("Path not found! path="+path)
            return []
            
        for node in nodes:
            node_path = self.GetNodePath(node, soup)           
            if node_path == path:
                #self.logger.debug("Path found! path="+path)
                found_soup_list.append(node)
        
        return found_soup_list
    
    '''Find soup using css selector
    '''    	
    def FindNodeBySelector(self, selector, attrs = None, soup = None):
        found_soup_list = []
        nodes = None
        if soup == None:
            nodes = self.soup.select(selector)
        else:
            nodes = soup.select(selector)
            
        if len(nodes) <= 0:
            self.logger.debug("Path not found! path="+selector)
            return None
        
        if attrs != None or len(attrs) > 0:
            for node in nodes:
                if self.MatchNode(node, node.name, attrs) == True:
                    found_soup_list.append(node)
        else:
            found_soup_list = nodes
        return found_soup_list           
        
    '''Find soup using css-selector like path
        bs not support xpath query now, using self implementation
        path example: html body div[id=1] a
        nodes(list): nodes in path without attr
        attrs(list): attr of each node, if no attr set to None
    '''               
    def FindNodeByPath(self, nodes, attrs, soup = None):        
        found_soup_list = {}
        root = (self.soup if soup == None else soup)
        nodes = root.findAll(name=nodes[-1], attrs=attrs[-1])
        for node in nodes:            
            found_soup_list[node] = node.parent
        
        for node_name,node_attr in reversed(zip(nodes[0:-1],attrs[0:-1])):
            for key,node in found_soup_list.items():
                if self.MatchNode(node, node_name, node_attr) == False:
                    del found_soup_list[key]
                else:
                    found_soup_list[key] = node.parent
            if len(found_soup_list) <= 0:
                self.logger.debug("Path not found! path="+nodes.join(" "))
                return []                                   
        
        return found_soup_list.keys()
        
    '''Loop to parse a range of code using xpath-like path
        start_path: start path to parse
        end_path: end path. None means parse all sibling
        level: parse start_path's sibling or children
    '''    
    def RangeParse(self, start_path, start_path_attrs, end_path, end_path_attrs, level, soup = None):
        found_soup_list = []
            
        end_nodes = None
        if end_path != None:
            end_nodes = self.FindNode(end_path, end_path_attrs, soup)
            if end_nodes == None or len(end_nodes) == 0:
                self.logger.debug("End node not found! path="+end_path)
                return found_soup_list
            if len(end_nodes) > 1:
                self.logger.warning("Found %d end node. path=%s" % (len(end_nodes), end_path))
                return found_soup_list
        
        if start_path != None and len(start_path) > 0:
            start_nodes = self.FindNode(start_path, start_path_attrs, soup)
            if start_nodes == None or len(start_nodes) == 0:
                self.logger.debug("Start node found! path="+start_path)
                return found_soup_list
            if len(start_nodes) > 1:
                self.logger.warning("Found %d start node. path=%s" % (len(start_nodes), start_path))
                if end_nodes != None:
                    self.logger.error("More than 1 start node and end node not None. Maybe error!")
                    return found_soup_list
        else:   #NOTE(chillbu): If start_path is empty, use current soup as start node, no need to find node
            start_nodes = [soup]
        
        if end_nodes == None:
            for index,start_node in enumerate(start_nodes):
                ret = self.__RangeParse__(start_node, None, level)
                found_soup_list.append(ret)
        else:
            found_soup_list.append(self.__RangeParse__(start_nodes[0], end_nodes[0], level))
        
        return found_soup_list

    ###################################################################################
    # 根据selector的范围抽取节点列表
    # start_selector：形如#sogou_ws_results > div > .aggrBox > div
    # @author：liuyongshuai
    ###################################################################################
    def RangeParseBySelector(self, start_selector,end_selector, level, soup = None):
        found_soup_list = []

        end_nodes = None
        if end_selector != None:
            end_nodes = self.FindNodeBySelector(end_selector,dict(),soup)
            if end_nodes == None or len(end_nodes) == 0:
                self.logger.debug("End node not found! path="+end_selector)
                return found_soup_list
            if len(end_nodes) > 1:
                self.logger.warning("Found %d end node. path=%s" % (len(end_nodes), end_selector))
                return found_soup_list

        if start_selector != None and len(start_selector) > 0:
            start_nodes = self.FindNodeBySelector(start_selector,dict(),soup)
            if start_nodes == None or len(start_nodes) == 0:
                self.logger.debug("Start node found! path="+start_selector)
                return found_soup_list
            if len(start_nodes) > 1:
                self.logger.warning("Found %d start node. path=%s" % (len(start_nodes), start_selector))
                if end_nodes != None:
                    self.logger.error("More than 1 start node and end node not None. Maybe error!")
                    return found_soup_list
        else:   #NOTE： If start_path is empty, use current soup as start node, no need to find node
            start_nodes = [soup]

        if end_nodes == None:
            for index,start_node in enumerate(start_nodes):
                ret = self.__RangeParse__(start_node, None, level)
                found_soup_list.append(ret)
        else:
            found_soup_list.append(self.__RangeParse__(start_nodes[0], end_nodes[0], level))

        return found_soup_list

    def FindFirstChildNode(self, soup, node_name, attrs = None):                
        for child in list(soup.children):
            if not isinstance(child, Tag):
                continue
            if self.MatchNode(child, node_name, attrs) == True:                
                return child        
        return None
    
    '''Compose self-define path for node
        example: html@body@div       
    ''' 
    def GetNodePath(self, soup, root = None):
        path = ""
        node = soup
        while 1:
            if node is not None:
                if node.name == "[document]" or node == root:
                    break
                if len(path) <= 0:
                    path = node.name
                else:
                    path = node.name + "@" + path
                node = node.parent
            else:
                break
        return path
        
    '''Compose selector path for node
        example: html body#id div
    ''' 
    def GetNodeSelector(self, soup):
        path = ""
        node = soup        
        while 1:
            if node is not None:
                if node.name == "[document]":
                    break
                    
                if "id" in node.attrs:
                    node_path = node.name+"#"+node["id"]
                elif "class" in node.attrs:
                    node_path = node.name+"[class="+ str(node["class"]) + "]"
                else:
                    node_path = node.name
                    
                if len(path) <= 0:
                    path = node_path
                else:
                    path = node_path + " " + path
                node = node.parent
            else:
                break
        return path
    
    '''Compose selector info for node
        example: node_name[id=test][class=test][sample=test]
    '''    
    def GetNodeInfo(self, node, attr_names = None):
        node_info = node.name
        attr_list = []
        if attr_names == None:
            for name,value in node.attrs.items():                
                attr_list.append("[%s=%s]" % (name, value))
        else:
            for name,value in node.attrs.items():
                if name not in attr_names:
                    attr_list.append("[%s=%s]" % (name, value))
        attr_list.sort()        
        return node_info+"".join(attr_list)                 
        
    '''Check if node match given node_name and attrs
    ''' 
    def MatchNode(self, node, node_name, attrs):
        if node.name != node_name:
            return False
        
        if attrs != None:
            for name,value in attrs.items():
                if value == None:   # name can not in node attrs
                    if name in node.attrs:
                        return False
                else:
                    if name not in node.attrs:
                        return False                
                    if not isinstance(node[name], types.ListType) and not value.match(node[name]):  #isinstance(value, retype)
                        return False
                    if isinstance(node[name], types.ListType) and not value.match("".join(node[name])):
                        return False
        return True                 
    
    '''Internal function
    '''
    def __RangeParse__(self, start, end, level):
        found_soup_list = []

        if level == "sibling":
            start = start.next_sibling
            if start == None:
                self.logger.debug("No sibling found for start path!")
                return found_soup_list
        elif level == "children":
            if len(list(start.children)) == 0:
                self.logger.debug("No children found for start path!")
                return found_soup_list            
            start = list(start.children)[0]
        else:
            self.logger.error("Unknown level(%s)!" % level)
            return found_soup_list
        
        if isinstance(start, Tag) == True:            
            found_soup_list.append(start) 
        for sibling in start.next_siblings:
            if sibling == end:
                self.logger.debug("Found end node, stop loop parse")
                break
            if not isinstance(sibling, Tag):
                continue        
            found_soup_list.append(sibling)            
        
        return found_soup_list
        
        
    def __GetLastNodeName__(self, path):        
        return path.split('@')[-1]         
      
      
class StructureParser:
    def __init__(self, logger):
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
    
    display_none_re = re.compile(u"display\\s*:\\s*none", re.S|re.I)
    decorate_tags = ['b','bdo','big','blockquote','br','cite','code','del','dfn','em','i','strong','font','samp','kbd','var','small','strike','tt','optgroup','option']
    '''Parse structure of given node
        div
         - div
          - div
           - div
         - span
         - div
       Structure: div|div@div|div@div@div|span|div
    '''       
    def Parse(self, root, engine):
        if engine == "baidu":
            return self.__StructureParseForBaidu__(root)
        else:
            return self.__StructureParseForCommon__(root)

    
    '''Compose self-define path for node
        example: html@body@div       
    '''
    def GetNodePath(self, soup, root = None):
        path = ""
        node = soup
        while 1:
            if node is not None:
                if node.name == "[document]" or node == root:
                    break
                if len(path) <= 0:
                    path = node.name
                else:
                    path = node.name + "@" + path
                node = node.parent
            else:
                break
        return path
    
    def __StructureParseForCommon__(self, root):
        #self.logger.debug("\n------ StructureParse ------")
        node_list = []
        structure = ""
        root_descendants = list(root.descendants)
        children_num = len(root_descendants)
        index = 0
        while index < children_num:
            child = root_descendants[index]
            #self.logger.debug("\nget_result child html : "+child.prettify().encode("gbk","ignore"))
            if not isinstance(child, Tag):
                index += 1                
                continue
            name = child.name
            #self.logger.debug("TAG = " + name)                        
            #跳过隐藏节点和非结构节点
            if name == "script" or name == "style":
                index += 1
                continue
            if "style" in child.attrs and self.display_none_re.match(child.attrs["style"]):
                index += 1
                continue           
                
            if name in self.decorate_tags and "class" not in child.attrs and "id" not in child.attrs:
                #无id与class的修饰节点
                if len(list(child.children)) == 0:    #叶子修饰节点
                    #self.logger.debug("------ Skip: decorate leaf node ------")
                    index += 1                
                    continue
                else:
                    is_keep = False
                    for node in child.descendants:
                        if not isinstance(node, Tag):
                            index += 1                
                            continue
                        if node.name not in self.decorate_tags or ("class" in node.attrs or "id" in node.attrs):
                            is_keep = True
                            break
                    if is_keep == True: #子节点不全是修饰节点
                        node_path = self.GetNodePath(child, root)
                        #self.logger.debug("NodePath=" + node_path)
                        structure = structure + "|" + node_path
                    else:   #子节点全是修饰节点
                        #self.logger.debug("------ Skip: all descendants decorate node ------")
                        index += 1                
                        continue
            else:
                node_path = self.GetNodePath(child, root)
                #self.logger.debug("NodePath=" + node_path)
                structure = structure + "|" + node_path
	    index += 1                

        return structure
    
    def __StructureParseForBaidu__(self, root):
        #self.logger.debug("\n------ StructureParse ------")
        node_list = []
        structure = ""
        root_descendants = list(root.descendants)
        children_num = len(root_descendants)
        index = 0
        while index < children_num:
            child = root_descendants[index]
            #self.logger.debug("\nget_result child html : "+child.prettify().encode("gbk","ignore"))
            if not isinstance(child, Tag):
                index += 1                
                continue
            name = child.name
            #self.logger.debug("TAG = " + name)                        
            #跳过隐藏节点和非结构节点
            if name == "script" or name == "style":
                index += 1
                continue
            if "style" in child.attrs and self.display_none_re.match(child.attrs["style"]):
                index += 1
                continue
            #NOTE@chillbu: 过滤节点
            if "class" in child.attrs:
                if child.attrs["class"] == "videotxticon" or child.attrs["class"] == "op_tieba2_icon op_tieba2_icon_pic" \
                  or child.attrs["class"] == "op_tieba2_icon op_tieba2_icon_good" or child.attrs["class"] == "op_car1_color":
                    self.logger.debug("SKIP class="+child.attrs["class"])
                    index += 1                
                    continue
                if child.attrs["class"] == "ecl-weigou-product-original-price":
                    self.logger.debug("SKIP class="+child.attrs["class"])
                    index += 3 
                    continue
                
            if name in self.decorate_tags and "class" not in child.attrs and "id" not in child.attrs:
                #无id与class的修饰节点
                if len(list(child.children)) == 0:    #叶子修饰节点
                    #self.logger.debug("------ Skip: decorate leaf node ------")
                    index += 1                
                    continue
                else:
                    is_keep = False
                    for node in child.descendants:
                        if not isinstance(node, Tag):
                            index += 1                
                            continue
                        if node.name not in self.decorate_tags or ("class" in node.attrs or "id" in node.attrs):
                            is_keep = True
                            break
                    if is_keep == True: #子节点不全是修饰节点
                        node_path = self.GetNodePath(child, root)
                        #self.logger.debug("NodePath=" + node_path)
                        structure = structure + "|" + node_path
                    else:   #子节点全是修饰节点
                        #self.logger.debug("------ Skip: all descendants decorate node ------")
                        index += 1                
                        continue
            else:
                node_path = self.GetNodePath(child, root)
                #self.logger.debug("NodePath=" + node_path)
                structure = structure + "|" + node_path
	    index += 1                

        return structure

class IDParser:
    def __init__(self, logger):
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
            
    sogou_type_id_full_re = re.compile(".*?id=['\"]?(sogou_vr|uigs)(_[a-zA-Z]+?)?_([\da-zA-Z]+?)($|_)")
    sogou_type_id_re = re.compile("^(sogou_vr|uigs)(_[a-zA-Z]+?)?_([\da-zA-Z]+?)($|_)")
    soso_type_id_full_re = re.compile(".*?['\"](w(\.[\w\d]+)+)['\"\s]")
    soso_type_id_re = re.compile(".*?(w(\.[\w\d]+)+)")
    SOSO_ID_SECTION_NUM = 4

    # GetTypeID return (full_id, analyse_id)
    def Parse(self, engine, url_tag, result_tag):
        match = None
        if "sogou" in engine:
            if url_tag != None and "id" in url_tag.attrs:
                match = self.sogou_type_id_re.match(url_tag["id"])                                
            if not match:
                self.logger.debug("No type id found in url node")
                if result_tag == None:
                    return (None, None)
                match = self.sogou_type_id_full_re.search(str(result_tag))
                if not match:
                    self.logger.debug("No type id found in result node")
                    return (None, None)
            type_id = match.group(3)
            full_type_id = match.group(1)+("" if match.group(2)==None else match.group(2))+"_"+match.group(3)
            if len(type_id) == 8 and type_id.isdigit() and type_id[0] != '2': #外部数据，不需要提取id
                return (full_type_id, full_type_id[:-2])
            return (full_type_id, full_type_id)        
        elif engine == "soso":
            if url_tag != None and "onclick" in url_tag.attrs:
                match = self.soso_type_id_re.match(url_tag["onclick"])            
            if not match:
                self.logger.debug("No type id found in url node")
                if result_tag == None:
                    return (None, None)
                match = self.soso_type_id_full_re.search(str(result_tag))
                if not match:
                    self.logger.debug("No type id found in result node")
                    return (None, None)
            type_id = match.group(1)
            type_id_arr = type_id.split('.')        
            return (type_id, '.'.join(type_id_arr[0:self.SOSO_ID_SECTION_NUM]))               

        return (None, None)

      
'''Enterance of the script
'''        
if __name__ == "__main__":
    test_case_file = "./base_parser_test.htm"
    if os.path.isfile(test_case_file) == False:
        print "Test case file not found! file="+test_case_file
        sys.exit(1)
        
    base_parser = BaseParser()
    html_string = open(test_case_file).read()
    base_parser.Parse(html_string, "utf-8")
    
    qc_selector = "html body div#wrapper div#main div#common_qc_container"
    print "\n######### Test case 1 ###########"
    print "Function: FindNode"
    print "Path: "+qc_selector
    found = base_parser.FindNodeBySelector(qc_selector)
    if found != None and len(found) > 0:
        print "Result: \n"+found[0].prettify()
    else:
        print "Failed Test"
        
    container_path = "html@body@div@div@div@div"
    container_attrs = {"class":"results"}
    print "\n######### Test case 2 ###########"
    print "Function: FindNodeByPath"
    print "Path: "+container_path
    print "Attrs: "+str(container_attrs)
    found = base_parser.FindNode(container_path, container_attrs)
    if found != None:
        print "Result: found "+str(len(found))+" matches"
    else:
        print "Failed Test"
        
    start_selector = "html body div#wrapper div#main div div[class=results]"
    print "\n######### Test case 3 ###########"
    print "Function: RangeParse"
    print "StartPath: "+start_selector
    found = base_parser.RangeParse(start_selector, None, None, None, level="children")
    if found != None:
        print "Result: Parser found "+str(len(found))+" container in loop"
        for index,container in enumerate(found):
            print "Result: Parser found "+str(len(container))+" children in loop"
            for index,node in enumerate(container):
                print "\t#%d\t%s" % (index, node.name)
    else:
        print "Failed Test"
        
    start_selector = "html body div#wrapper div#main div div[class=results]"
    end_selector = "html body div#wrapper div#main div div[class=results] h2[class=qcrst]"
    print "\n######### Test case 4 ###########"
    print "Function: RangeParse"
    print "StartPath: "+start_selector
    print "EndPath: "+end_selector
    found = base_parser.RangeParse(start_selector, None, end_selector, None, level="children")
    if found != None:
        print "Result: Parser found "+str(len(found))+" container in loop"
        for index,container in enumerate(found):
            print "Result: Parser found "+str(len(container))+" children in loop"
            for index,node in enumerate(container):
                print "\t#%d\t%s" % (index, node.name)
    else:
        print "Failed Test"
            
    start_selector = "html@body@div@div@div@div@h2[class=qcrst]"
    print "\n######### Test case 5 ###########"
    print "Function: RangeParse"
    print "StartPath: "+start_selector
    found = base_parser.RangeParse(start_selector, None, None, None, level="sibling")
    if found != None:
        print "Result: Parser found "+str(len(found))+" container in loop"
        for index,container in enumerate(found):
            print "Result: Parser found "+str(len(container))+" children in loop"
            for index,node in enumerate(container):
                print "\t#%d\t%s" % (index, node.name)
    else:
        print "Failed Test"
