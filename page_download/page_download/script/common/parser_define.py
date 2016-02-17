#!/usr/local/bin/python
# coding=gbk
#######################################################
#name:      parser_define.py
#author:    chillbu
#create:    20130408
#modify:    20130418 - ����SearchResult����
#
#desc: ������������������ݽṹ����Ҫ���ݽṹ��
#      PageElement - ������õ���ҳ��Ԫ��
#      SearchResult - ���������Ϣ
#      PageInfo - ������õ���ҳ����Ϣ    
#
#######################################################                                

'''ϵͳ������Item ID
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

#����������
AD_TYPE_PZL = 1 #Ʒ�ƹ��
AD_TYPE_TOP = 2 #����һ����
AD_TYPE_BOTTOM = 3 #�ײ�һ����
AD_TYPE_NORMAL = 0 #�ǹ�棬�������

AD_TYPE_LIST = [AD_TYPE_PZL,AD_TYPE_TOP,AD_TYPE_BOTTOM,AD_TYPE_NORMAL]

                                
class PageInfo:
    '''����ҳ����Ϣ
        ����ҳ��Ļ�����Ϣ
    '''  
    def __init__(self):
        self.file_path = ""
        self.container_paths = []
        self.container_num = 0
        self.result_num = 0
        self.known_result_num = 0
        self.url_result_num = 0
        self.known_url_result_num = 0
        self.ad_pzl_num = 0 #Ʒ�ƹ������
        self.ad_top_num = 0 #����һ��������
        self.ad_bottom_num = 0 #�ײ�һ��������

class SearchResult:
    '''���������Ϣ
        ������������Ļ�����Ϣ: url, title, abs, type
    '''
    def __init__(self):
        self.raw_url = ""
        self.url = ""
        self.docid = ""
        self.title = ""
        self.abstract = ""
        self.type = ""          #��Ʒ��̬ϵͳ����
        self.type_desc = ""     #�������
        
    def ToString(self):
        ret = "type: %s\ntype_desc: %s\nraw_url: %s\nurl: %s\ndocid: %s\ntitle: %s\nabstract: %s\n" \
                % (str(self.type), self.type_desc.encode('gbk','ignore'), self.raw_url.encode('gbk','ignore'), self.url.encode('gbk','ignore'), str(self.docid), self.title.encode('gbk','ignore'), self.abstract.encode('gbk','ignore'))
	return ret
        
class PageElement:
    '''����ҳ��Ԫ��
        ���������ҳ��Ԫ����Ϣ
    '''
    def __init__(self):
        self.rank = 0               #�����ţ�����һ����Ԫ��
        self.index = 0              #Ψһ���
        self.parent_index = 0       #���ڵ���
        self.container_id = 0       #����container���
        self.item_id = 0            #���е�item����id
        self.item_desc = ""         #���е�item��������
        self.parent_item_id = 0     #parent���е�item����id
        self.children = []          #�ӽڵ�����
        self.path = ""              #css selector: html body div#result        
        self.analyse_path = ""      #����·������selector���д����õ���·��
        self.structure = ""         #�ڵ�ṹ
        self.analyse_structure = "" #�����ṹ����������Ľڵ�ṹ
        self.node_info = ""         #tag�����������Ϣ: tagname#id[class=][attr=]...
        self.type_str = ""          #����string������һ�������ȡ
        self.analyse_type_str = ""  #��������string
        self.bs_tag = None          #��Ӧbs�е�tag����
        self.url_tag = None         #URL��tag����       
        self.search_result = None   #���������Ϣ
        self.is_eval = True         #������ҳ���У��Ƿ���������
        self.is_keep = True         #������ҳ���У��Ƿ���
        self.is_rank = True         #������ҳ���У�������λ����Ϣ
        self.is_save = True         #�Ƿ񱣴浽���ݿ�
        self.has_script = False     #Ԫ�����Ƿ����script
        self.ad_type = 0            #����Ĺ�����ͣ�0Ϊ���������1Ʒ�ƹ�桢2����һ���桢3�ײ�һ����
        
    def ToString(self):        
        result_string = "************* PageElement Info ******************\n" \
                "rank: %s\nindex: %s\nparent_index: %s\nitem_id: %s\nitem_desc: %s\nparent_item_id: %s\n" \
                "path: %s\nnode_info: %s\nanalyse_path: %s\nstructure: %s\n" \
                "analyse_structure: %s\nis_eval: %s\nis_keep: %s\nis_rank: %s\nis_save: %s\n" \
                "has_script: %s\n[search_result]\n%s" \
                "bs_tag:%s\nad_type:%s\n" \
                % (str(self.rank), str(self.index), str(self.parent_index), str(self.item_id), str(self.item_desc), str(self.parent_item_id), \
                str(self.path), self.node_info.encode('gbk'), str(self.analyse_path), str(self.structure), \
                str(self.analyse_structure), str(self.is_eval), str(self.is_keep), str(self.is_rank), str(self.is_save), \
                str(self.has_script), ("\n" if self.search_result == None else self.search_result.ToString()), \
                str(self.bs_tag),str(self.ad_type))
        result_string += "[children]\n"
        for child in self.children:
            result_string += child.ToString()
        result_string += "\n"
            
        return result_string

'''����������        
'''                
class PathRule:
    '''·�����򣬿��ܰ����Ķ���:
        1. �ҵ�ָ��·���Ľڵ�
        2. �жϽڵ��Ƿ�������ƥ��
        3. ��ƥ���������Ϣ��ȡ
    '''
    def __init__(self):
        self.name = ""
        self.path = ""
        self.node_name = ""
        self.node_attr = {}         #������������ֵΪ���ַ���������Ϊ�����Բ�����
        self.node_attr_re = {}       
        self.text = ""              #�ڵ���ı�����
        self.text_re = None
        self.structure = ""         #�ڵ�Ľṹ����
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
    '''��Χ���򣬿��ܰ����Ķ���:
        1. �ҵ�ָ����ʼ·���ڵ���ֹͣ·���ڵ�
        2. ��ָ���Ĳ�α�����ȡ�ӽڵ�
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
    '''Item���򣬿��ܰ����Ķ���:
        1. �жϵ�ǰ�ڵ��Ƿ��������
        2. ��ȡ��Ϣ
        3. ���ǩ
    '''
    def __init__(self):
        self.id = 0
        self.desc = ""
        self.name = ""        
        self.container_id = 0       #container������Ĭ��Ϊ0����ʾ���ж�
        self.parent_item_id = 0     #��Item������Ĭ��Ϊ0����ʾ���ж�
        self.cond_rules = []        #·������
        self.get_rules = []         #��Ϣ��ȡ·��
        self.container_rule = None  #�ӽ����ȡ����
        self.set_structure = None   #�Զ���ṹ����
        self.is_eval = True         #������ҳ���У��Ƿ���������
        self.is_keep = True         #������ҳ���У��Ƿ���
        self.is_rank = True         #������ҳ���У�������λ����Ϣ
        self.is_save = True         #�Ƿ񱣴浽���ݿ�
        self.is_auto_parse = False  #�Ƿ�ʹ���Զ�����
        self.ad_type = 0            #�������
        
    def __str__(self):
        cond_rule_string = ""
        get_rule_string = ""
        for index,cond_rule in enumerate(self.cond_rules):
            cond_rule_string += "#%d %s" % (index, str(cond_rule))
        for index,get_rule in enumerate(self.get_rules):
            get_rule_string += "#%d %s" % (index, str(get_rule))
        return "[ItemRule-%s]\nid: %s\ndesc: %s\nset_structure: %s\ncontainer_id: %s\nparent_item_id: %s\nis_auto_parse: %s\nis_eval: %s\nis_keep: %s\nis_rank: %s\nis_rank: %s\ncond_rules: %s\nget_rules: %s\ncontainer_rule: %s\nad_type:%s\n" \
                % (self.name, str(self.id), self.desc, self.set_structure, str(self.container_id), str(self.parent_item_id), str(self.is_auto_parse), str(self.is_eval), str(self.is_keep), str(self.is_rank), str(self.is_save), cond_rule_string, get_rule_string, str(self.container_rule),str(self.ad_type))
                
