#!/usr/local/bin/python
# -*- coding: utf-8 -*-
#######################################################
#name:      parser_define.py
#author:    chillbu
#create:    20130408
#modify:    20130418 - 增加SearchResult定义
#
#desc: 定义分析程序的相关数据结构，主要数据结构：
#      PageElement - 分析后得到的页面元素
#      SearchResult - 搜索结果信息
#      PageInfo - 分析后得到的页面信息    
#
#######################################################                                

'''系统保留的Item ID
'''             
ITEM_ID_STYLE = 101
ITEM_ID_SCRIPT = 102
ITEM_ID_BR = 103
ITEM_ID_DISPLAY_NONE = 104
ITEM_ID_NO_TEXT = 105
ITEM_ID_LINK = 106
NOT_ANCHOR_ITEMS_NAME = ["style", "script", "br", "link"]
NOT_ANCHOR_ITEMS = [ITEM_ID_STYLE,ITEM_ID_SCRIPT,ITEM_ID_BR, ITEM_ID_LINK]
NOT_DISPLAY_ITEMS = [ITEM_ID_STYLE,ITEM_ID_SCRIPT,ITEM_ID_BR,ITEM_ID_DISPLAY_NONE,ITEM_ID_NO_TEXT,ITEM_ID_LINK]

#结果广告类型
AD_TYPE_PZL = 1 #品牌广告
AD_TYPE_TOP = 2 #顶部一般广告
AD_TYPE_BOTTOM = 3 #底部一般广告
AD_TYPE_NORMAL = 0 #非广告，正常结果

AD_TYPE_LIST = [AD_TYPE_PZL,AD_TYPE_TOP,AD_TYPE_BOTTOM,AD_TYPE_NORMAL]

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

                                
class PageInfo:
    '''基本页面信息
        保存页面的基本信息
    '''  
    def __init__(self):
        self.file_path = ""
        self.container_paths = []
        self.container_num = 0
        self.result_num = 0
        self.known_result_num = 0
        self.url_result_num = 0
        self.known_url_result_num = 0
        self.ad_pzl_num = 0 #品牌广告数量
        self.ad_top_num = 0 #顶部一般广告数量
        self.ad_bottom_num = 0 #底部一般广告数量

class SearchResult:
    '''搜索结果信息
        保存搜索结果的基本信息: url, title, abs, type
    '''
    def __init__(self):
        self.raw_url = ""
        self.url = ""
        self.docid = ""
        self.title = ""
        self.abstract = ""
        self.type = ""          #产品形态系统分类
        self.type_desc = ""     #类别描述
        
    def ToString(self):
        ret = "type: %s\ntype_desc: %s\nraw_url: %s\nurl: %s\ndocid: %s\ntitle: %s\nabstract: %s\n" \
                % (str(self.type), self.type_desc, self.raw_url, self.url, str(self.docid), self.title, self.abstract)
	return ret
        
class PageElement:
    '''基本页面元素
        保存基本的页面元素信息
    '''
    def __init__(self):
        self.rank = 0               #结果序号，过滤一定的元素
        self.index = 0              #唯一编号
        self.parent_index = 0       #父节点编号
        self.container_id = 0       #顶层container编号
        self.item_id = 0            #命中的item规则id
        self.item_desc = ""         #命中的item规则描述
        self.parent_item_id = 0     #parent命中的item规则id
        self.children = []          #子节点数组
        self.path = ""              #css selector: html body div#result        
        self.analyse_path = ""      #分析路径，对selector进行处理后得到的路径
        self.structure = ""         #节点结构
        self.analyse_structure = "" #分析结构，经过处理的节点结构
        self.node_info = ""         #tag对象的描述信息: tagname#id[class=][attr=]...
        self.type_str = ""          #类型string，根据一定规则抽取
        self.analyse_type_str = ""  #分析类型string
        self.bs_tag = None          #对应bs中的tag对象
        self.url_tag = None         #URL的tag对象       
        self.search_result = None   #搜索结果信息
        self.is_eval = True         #在评测页面中，是否用于评测
        self.is_keep = True         #在评测页面中，是否保留
        self.is_rank = True         #在评测页面中，计算结果位置信息
        self.is_save = True         #是否保存到数据库
        self.ad_type = 0            #结果的广告类型，0为正常结果，1品牌广告、2顶部一般广告、3底部一般广告
        
    def ToString(self):        
        result_string = "************* PageElement Info ******************\n" \
                "rank: %s\nindex: %s\nparent_index: %s\nitem_id: %s\nitem_desc: %s\nparent_item_id: %s\n" \
                "path: %s\nnode_info: %s\nanalyse_path: %s\nstructure: %s\n" \
                "analyse_structure: %s\nis_eval: %s\nis_keep: %s\nis_rank: %s\nis_save: %s\n" \
                "[search_result]\n%s" \
                "bs_tag:%s\nad_type:%s\n" \
                % (str(self.rank), str(self.index), str(self.parent_index), str(self.item_id), str(self.item_desc), str(self.parent_item_id), \
                str(self.path), self.node_info, str(self.analyse_path), str(self.structure), \
                str(self.analyse_structure), str(self.is_eval), str(self.is_keep), str(self.is_rank), str(self.is_save), \
                ("\n" if self.search_result == None else self.search_result.ToString()), \
                str(self.bs_tag),str(self.ad_type))
        result_string += "[children]\n"
        for child in self.children:
            result_string += child.ToString()
        result_string += "\n"
            
        return result_string

'''解析规则定义        
'''                
class PathRule:
    '''路径规则，可能包含的动作:
        1. 找到指定路径的节点
        2. 判断节点是否与条件匹配
        3. 若匹配则进行信息抽取
    '''
    def __init__(self):
        self.name = ""
        self.path = ""
        self.node_name = ""
        self.node_attr = {}         #属性条件，若值为空字符串，条件为该属性不存在
        self.node_attr_re = {}       
        self.text = ""              #节点的文本条件
        self.text_re = None
        self.structure = ""         #节点的结构条件
        self.structure_re = None
        self.gets = []
    
    def GetPath(self):
        string = ""
        if self.node_name != "":
            string = self.node_name
        else:
            string = self.path
        for name,value in self.node_attr.items():
            string += "[%s=%s]" % (name, value)
        return string
    
    def __str__(self):
        return "[PathRule-%s]\npath: %s\nnode_name: %s\nnode_attr: %s\ntext: %s\nstructure: %s\ngets: %s\n" \
                % (self.name, self.path, self.node_name, str(self.node_attr), self.text, self.structure, str(self.gets))

class RangeRule:
    '''范围规则，可能包含的动作:
        1. 找到指定起始路径节点与停止路径节点
        2. 按指定的层次遍历抽取子节点
    '''
    def __init__(self):
        self.id = 0
        self.desc = ""
        self.name = ""
        self.start_path_rule = None
        self.end_path_rule = None
        self.level = ""
        self.method = ""
        
    def __str__(self):
        return "[RangeRule-%s]\nid: %s\ndesc: %s\nlevel: %s\nstart: %s\nend: %s\n" \
                % (self.name, str(self.id), self.desc, self.level, str(self.start_path_rule), str(self.end_path_rule))
                
class ItemRule:
    '''Item规则，可能包含的动作:
        1. 判断当前节点是否符合条件
        2. 抽取信息
        3. 打标签
    '''
    def __init__(self):
        self.id = 0
        self.desc = ""
        self.name = ""        
        self.container_id = 0       #container条件，默认为0，表示不判断
        self.parent_item_id = 0     #父Item条件，默认为0，表示不判断
        self.cond_rules = []        #路径条件
        self.get_rules = []         #信息抽取路径
        self.container_rule = None  #子结果抽取配置
        self.set_structure = None   #自定义结构特征
        self.is_eval = True         #在评测页面中，是否用于评测
        self.is_keep = True         #在评测页面中，是否保留
        self.is_rank = True         #在评测页面中，计算结果位置信息
        self.is_save = True         #是否保存到数据库
        self.is_auto_parse = False  #是否使用自定分析
        self.ad_type = 0            #广告类型
        
    def __str__(self):
        cond_rule_string = ""
        get_rule_string = ""
        for index,cond_rule in enumerate(self.cond_rules):
            cond_rule_string += "#%d %s" % (index, str(cond_rule))
        for index,get_rule in enumerate(self.get_rules):
            get_rule_string += "#%d %s" % (index, str(get_rule))
        return "[ItemRule-%s]\nid: %s\ndesc: %s\nset_structure: %s\ncontainer_id: %s\nparent_item_id: %s\nis_auto_parse: %s\nis_eval: %s\nis_keep: %s\nis_rank: %s\nis_rank: %s\ncond_rules: %s\nget_rules: %s\ncontainer_rule: %s\nad_type:%s\n" \
                % (self.name, str(self.id), self.desc, self.set_structure, str(self.container_id), str(self.parent_item_id), str(self.is_auto_parse), str(self.is_eval), str(self.is_keep), str(self.is_rank), str(self.is_save), cond_rule_string, get_rule_string, str(self.container_rule),str(self.ad_type))
                
