#!/usr/bin/python
# -*- coding: utf-8 -*-

################################################################
# 周三全天的sogou和baidu广告的统计需求
# sogou_query_num：sogou的总PV数
# baidu_query_num：baidu的总PV数
# sogou_ad_query_num：sogou的那些PV里带广告的数量
# baidu_ad_query_num：baidu那里带广告的数量
# sogou_ad_total：sogou的广告总条数（仅顶部一般广告）
# baidu_ad_total：baidu的广告总条数（由广告部门的指定）
# sogou_coverage：sogou的广告覆盖率，即sogou_ad_query_num/sogou_query_num
# baidu_coverage：baidu的广告覆盖率，即baidu_ad_query_num/baidu_query_num
# sogou_depth：sogou的广告深度，即sogou_ad_total/sogou_ad_query_num
# baidu_depth：baidu的广告深度，即baidu_ad_total/baidu_ad_query_num
################################################################

import sys,string
PY3 = sys.version_info[0] == 3

if PY3:
    def u(s):
        return s
else:
    def u(s):
        return s.decode('utf-8')

import IP
import urllib,os,time,simplejson
from utils.utils import *
from utils.DB import *
from sogou.conf import *
reload(sys)
sys.setdefaultencoding('utf8')

prov_code = "省".decode()
city_code = "市".decode()
skip1 = "自治区".decode()
skip2 = "壮族".decode()
skip3 = "回族".decode()
skip4 = "维吾尔".decode()

########################################################
# 统计sogou的顶部广告数目及query数目
########################################################
def sogou_ad(ad_log_file):
    final_ret = dict()
    fp = open(ad_log_file,"r")
    for line in fp:
        line = line.strip()
        try:
            items = simplejson.loads(line,strict=False)
        except Exception,args:
            print "sogou_ad",args
            continue
        html_file = items["html_file"]
        query = items["query"]
        province = items["prov_city"].split("-")[0]
        province = province.rstrip(prov_code)
        province = province.rstrip(city_code)
        province = province.rstrip(skip1)
        province = province.rstrip(skip2)
        province = province.rstrip(skip3)
        province = province.rstrip(skip4)

        #get uuid
        bn = os.path.basename(html_file).rstrip(".html")
        uuid = bn[bn.find("-")+1:]
        if province not in final_ret:
            final_ret[province] = dict()
        if uuid not in final_ret[province]:
            final_ret[province][uuid] = dict({
                   "query":query.replace(",","，"),
                   "ad2_num":0,
                   "html_file":html_file})
        final_ret[province][uuid]["ad2_num"] += items["ad_top_num"]


    #output by query and province
    stat_by_province = dict()
    for prov in final_ret:
        if prov not in stat_by_province:
            stat_by_province[prov] = dict({
                   "query_num":0,
                   "ad2_query_num":0,
                   "ad2_num":0,
            })
        for tmp in final_ret[prov]:
            stat_by_province[prov]["query_num"] += 1
            if final_ret[prov][tmp]["ad2_num"] > 0:
                stat_by_province[prov]["ad2_query_num"] += 1
            stat_by_province[prov]["ad2_num"] += final_ret[prov][tmp]["ad2_num"]
    fp.close()
    return stat_by_province

########################################################
# 统计baidu的广告数据及query总量
########################################################
def baidu_ad(ad_log_file_list):
    final_ret = dict()
    for ad_log_file in ad_log_file_list:
        fp = open(ad_log_file,"r")
        for line in fp:
            line = line.lstrip("\xef\xbb\xbf")
            line = line.strip(string.whitespace)
            item = line.split("\4")
            if len(item) != 21:
                continue
            timestamp = item[0].strip(string.whitespace)
            try:
                timestamp = float(timestamp)
            except:
                print "time convert error"
                continue
            tm = time.localtime(float(timestamp))
            query = item[2]
            #query = query.replace("\t"," ")
            ip = item[3].strip(string.whitespace)
            ad_list = item[8].strip(string.whitespace)
            actm = time.strftime("%Y-%m-%d %H:%M:%S", tm)
            province = "unknown"
            if ip and len(ip) > 0:
                ip_str = IP.find(ip)
                ip_str = ip_str.split("\t") if ip_str else ["","unknown"]
                province = ip_str[1]
            province = province.rstrip(prov_code)
            province = province.rstrip(city_code)
            province = province.rstrip(skip1)
            province = province.rstrip(skip2)
            province = province.rstrip(skip3)
            province = province.rstrip(skip4)
            if province not in final_ret:
                final_ret[province] = dict({
                        "query_num":0,
                        "ad_query_num":0,
                        "no_ad_query_num":0,
                        "ad_total":0
                })
            ad_type_dict = dict()
            ad_type_dict[0] = ad_type_dict[1] = 0
            ad_type_list = ad_list.split("\3")
            if len(ad_type_list) <= 0:
                print "ad type list error"
                continue
            for ad_type in ad_type_list:
                try:
                    if len(ad_type)<=0 or int(ad_type) > 1:
                        continue
                except:
                    continue
                ad_type_dict[int(ad_type)] += 1
            total_ad = ad_type_dict[0] + ad_type_dict[1]
            final_ret[province]["query_num"] += 1
            if total_ad > 0:
                final_ret[province]["ad_query_num"] += 1
            else:
                final_ret[province]["no_ad_query_num"] += 1
            final_ret[province]["ad_total"] += total_ad
        fp.close()
    return final_ret

####################################################################################
#统计信息入库，给绿皮系统用的
#字段顺序为：date,prov_area,sogou_query_num,baidu_query_num,sogou_ad_query_num,baidu_ad_query_num,
# sogou_ad_total,baidu_ad_total,sogou_coverage,baidu_coverage,sogou_depth,baidu_depth
####################################################################################
db=None
def insert_db(ret_list,tm_str):
    global db
    if db == None:
        db = DB(user="phpmyadmin",passwd="123456",host="10.11.195.131",db="ad_stat",port=3306,charset="latin1")
    update_data = dict({
        "sogou_query_num":ret_list[1],
        "baidu_query_num":ret_list[2],
        "sogou_ad_query_num":ret_list[3],
        "baidu_ad_query_num":ret_list[4],
        "sogou_ad_total":ret_list[5],
        "baidu_ad_total":ret_list[6],
        "sogou_coverage":ret_list[7],
        "baidu_coverage":ret_list[8],
        "sogou_depth":ret_list[9],
        "baidu_depth":ret_list[10],
    })
    insert_data = dict({
        "date":tm_str,
        "prov_area":ret_list[0].encode("GBK"),
    })
    insert_data.update(update_data)
    db.insert_update("ad_province",insert_data,update_data)
    return  True


#-------------------------------------------------------------------------------------
######################################################################################
# 主入口，程序有点长，逻辑有点乱，百度的广告文件可以有多个
######################################################################################
#-------------------------------------------------------------------------------------
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print "USAGE: %s %s %s" % (sys.argv[0],"sogou_log_file","baidu_log_file")
        sys.exit()
    sogou_file = sys.argv[1]
    if not os.path.isfile(sogou_file):
        print "invalid sogou log file"
        sys.exit()
    baidu_file = sys.argv[2:]
    if len(baidu_file) <= 0:
        print "invalid baidu log file"
        sys.exit()
    tmp=os.path.dirname(sogou_file)
    tm_str=os.path.basename(tmp)

    #开始统计sogou和baidu的数据
    sogou_ret = sogou_ad(sogou_file)
    baidu_ret = baidu_ad(baidu_file)

    #拉出所有的省份，为的是让baidu和sogou的省份对齐
    all_prov = set(sogou_ret.keys()+baidu_ret.keys())

    #写入文件一份
    fin_file = "%s/sogou_baidu_%s.csv" % (tm_str,sogou_conf["stat_data_dir"].rstrip("/"))
    utils.mk_sure_dir(fin_file)
    fp = open(fin_file,"w+")
    fp.write("date,prov_area,sogou_query_num,baidu_query_num,sogou_ad_query_num,baidu_ad_query_num,"
             "sogou_ad_total,baidu_ad_total,sogou_coverage,baidu_coverage,sogou_depth,baidu_depth\n")

    #这里存的是最终的结果
    final_ret = list()
    sum_stat = list()
    for i in range(11):
        sum_stat.append(0)
    #遍历所有的省份，对不存在该省的，补0之
    for prov in all_prov:
        ostr = list()
        ostr.append(prov)
        for i in range(10):
            ostr.append(0)
        if prov in sogou_ret: #sogou各省的数据
            ostr[1] = sogou_ret[prov]["query_num"]
            ostr[3] = sogou_ret[prov]["ad2_query_num"]
            ostr[5] = sogou_ret[prov]["ad2_num"]
        if prov in baidu_ret: #baidu各省的数据
            ostr[2] = baidu_ret[prov]["query_num"]
            ostr[4] = baidu_ret[prov]["ad_query_num"]
            ostr[6] = baidu_ret[prov]["ad_total"]
        for i in range(1,7): #汇总数据的累加
            sum_stat[i] += ostr[i]
        #计算覆盖率、深度等
        if ostr[3] > 0:
            ostr[7] = float(ostr[3])/float(ostr[1])
        if ostr[5] > 0:
            ostr[9] = float(ostr[5])/float(ostr[3])
        if ostr[4] > 0:
            ostr[8] = float(ostr[4])/float(ostr[2])
        if ostr[6] > 0:
            ostr[10] = float(ostr[6])/float(ostr[4])
        final_ret.append(ostr)
    #统计汇总信息
    if sum_stat[3] > 0:
        sum_stat[7] = float(sum_stat[3])/float(sum_stat[1])
    if sum_stat[5] > 0:
        sum_stat[9] = float(sum_stat[5])/float(sum_stat[3])
    if sum_stat[4] > 0:
        sum_stat[8] = float(sum_stat[4])/float(sum_stat[2])
    if sum_stat[6] > 0:
        sum_stat[10] = float(sum_stat[6])/float(sum_stat[4])

    #排序
    sum_stat[0]="汇总"
    final_ret.append(sum_stat)
    final_ret.sort(lambda x,y:cmp(x[1],y[1]))
    final_ret.reverse()
    #排序并输出
    oret = enumerate(final_ret)
    for index,ol in oret:
        fstr = tm_str+","+str(ol[0]) + ","
        for i in range(1,11):
            fstr += str(ol[i]) + ","
        fstr = fstr.rstrip(",")
        fp.write(fstr+"\n")
        if index <= 10:
            insert_db(ol,tm_str)

    fp.close()
    
    
