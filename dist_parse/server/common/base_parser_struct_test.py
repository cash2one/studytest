#!/usr/local/bin/python
# coding=gbk
#######################################################
# Copyright (c) 2012, Tencent Inc.
# All rights reserved
#
# Author: Manzhao BU <chillbu@tencent.com>
#
# Desc: 用于测试base_parser中的StructureParse功能
#
#######################################################

import re
from base_parser import BaseParser

html_string = "<div id=\"uigs_1d32213_1\" onclick=\"st_get(this,'w.3412.')\" class=\"result\"> \
          <a class=\"result_title\" href=\"../pu=sz%401320_2001%2Cusm%401%2Cta%40iphone_1_5.1_3_534/w=0_10_%E5%9B%9B%E5%8D%83%E7%9A%84%E6%89%8B%E6%9C%BA/t=iphone/l=3/tc?ref=www_iphone&amp;lid=13968195908955033102&amp;order=6&amp;vit=osres&amp;tj=www_normal_6_0_10&amp;m=8&amp;cltj=cloud_title&amp;dict=2&amp;sec=29314&amp;di=bcedb3e4388b7ea0&amp;bdenc=1&amp;nsrc=Co01KwEptyoA_yixCFOxXnANedT62v3IEQGG_8sFQWn5nk_qva02HdFxUXKhVinGUpSwdoTXxBt8wX_u0VAn8BF_t_s0tnN8nyC\">三星i9100行货将破<em>四千</em> 热门<em>手机</em>价格预测 <em>手机</em>资讯 35..</a> \
          <div class=\"result_title_abs\"> \
            <span class=\"gray\">2011年9月16日 -?</span>大屏超薄索尼爱立信LT15i价格回升 索尼爱立信LT15i作为一款主打超薄智能手机... 索尼爱立信<em>的手机</em>一直是以造型独特来吸引消费者,就像这款拥有8.7毫米纤薄... \
          </div> \
          <div id=\"sogou_vr_123213_1\" ss_c=\"w.r\" class=\"result_snippet\"></div><span class=\"site\">www.3533.com</span>?<span class=\"size\">13k</span>? \
        </div>"

html_string_1 = "<div><a><em>222<div><em>111</em></div></em><div><div></div></div></a></div><div><a></a></div>"
html_string_2 = "<div><em><em><div></div></em></em></div>"
html_string_3 = "<div><a></a><a></a></div><span></span>"

'''Enterance of the script
'''        
if __name__ == "__main__":
    base_parser = BaseParser()
    soup = base_parser.Parse(html_string, "gbk")
    #print base_parser.StructureParse(soup)
    sogou_type_id_re = re.compile(".*?id=['\"]?(sogou_vr|uigs)_([\d\w]+)_[\d]+['\"]?")	
    soso_type_id_re = re.compile(".*?['\"](w(\.[\w\d]+)+)['\"\s]")
    #match = sogou_type_id_re.match(str(soup))
    match = soso_type_id_re.match(str(soup))
    print match.groups()
