#!/usr/local/bin/python
# -*- coding: utf-8 -*-

########################################################################################
# 给xueying统计广告评测用的，共产生4个任务，并发给xueying两个excel文件，主要包括：
# 1、09 ~ 11点的所有PV中，抽200个，分别将带广告的及不带广告的前三条导系统里，并发xueying一份
# 2、09 ~ 11点北上广三市的带广告的PV中，抽100个，操作同上（都包括品牌广告）
# 3、所有的数据都要 09:00 ~ 11:00 这两个小时的
########################################################################################

import IP
import urllib,os,time,simplejson
from utils.utils import *
from utils.DB import *
from sogou.conf import *
import hashlib
import copy
reload(sys)
sys.setdefaultencoding('utf8')

#抽全量的200PV带广告及不带广告的数据文件，导入系统用，要求GBK编码
all200_with_ad="ad_monitor_xueying_all_200_with_ad_%s.dat"
all200_no_ad="ad_monitor_xueying_all_200_no_ad_%s.dat"
all200 = list()
all200_with_ad_dict = dict()
all200_no_ad_dict = dict()

#北上广的带广告及不带广告的导入数据文件，要求也是GBK编码
bsg100_with_ad="ad_monitor_xueying_bsg_100_with_ad_%s.dat"
bsg100_no_ad="ad_monitor_xueying_bsg_100_no_ad_%s.dat"
bsg100 = list()
bsg100_with_ad_dict = dict()
bsg100_no_ad_dict = dict()
city_list = list(["北京","上海","广州"])
city_str = "市".decode()

#最后要发送给xueying的文件
task200_file = "xueying_all_200_%s.csv"
bsg100_file="xueying_bsg_100_%s.csv"

#快照地址样式
snapshot_url="http://10.11.195.131/getsnapshot?tid=%s&query=%s&page_type=page_search&engine_id=1"
rsync_cmd = "rsync %s root@10.11.195.131:/search/oss/new/crond/autotask/datalog/"

# 统计 09:00 ~ 11:00 的以下数据，全量的
allQuery_in_9_11 = 0 #总query数
ad2Query_in_9_11 = 0  #带顶部一般广告的query数
ad1_ad2Query_in_9_11 = 0 #只要有顶部广告及品牌广告数

# 统计 09:00 ~ 11:00 的以下数据，北上广的
bsg_Query_in_9_11 = 0 #总query数
bsg_ad2Query_in_9_11 = 0  #带顶部一般广告的query数
bsg_ad1_ad2Query_in_9_11 = 0 #只要有顶部广告及品牌广告数


timestamp_9 = 0 #9点的时间戳
timestamp_11 = 0 #11点整的时间戳

################################################################
# 统计广告信息，遍历结果文件等于说，包括品牌广告和顶部一般广告
################################################################
def get_sogou_stat(log_file):
    tmp=os.path.dirname(ret_file)
    tm_str=os.path.basename(tmp) #20140813格式的
    global all200,bsg100,allQuery_in_9_11,ad2Query_in_9_11,ad1_ad2Query_in_9_11,\
        timestamp_9,timestamp_11,bsg_Query_in_9_11,bsg_ad2Query_in_9_11,bsg_ad1_ad2Query_in_9_11

    #当天，9点整的时间戳
    timestamp_9 = time.strptime("%s-%s-%s %s:%s:%s" % (
        tm_str[0:4],tm_str[4:6],tm_str[6:],"09","00","00"
    ), "%Y-%m-%d %H:%M:%S")
    timestamp_9 = int(time.mktime(timestamp_9))

    #当天，11点的时间戳
    timestamp_11 = time.strptime("%s-%s-%s %s:%s:%s" % (
        tm_str[0:4],tm_str[4:6],tm_str[6:],"11","00","00"
    ), "%Y-%m-%d %H:%M:%S")
    timestamp_11 = int(time.mktime(timestamp_11))

    #所有的Query列出来
    all_query = dict()
    bsg_query = dict()
    fp = open(log_file,"r")
    for line in fp:
        line = line.strip()
        try:
            items = simplejson.loads(line,strict=False)
        except Exception,args:
            print "sogou_ad",args
            continue

        #只要 9:00 ~ 11:00 这两个小时的
        timeArray = time.strptime(items["time"], "%Y-%m-%d %H:%M:%S")
        timestamp = int(time.mktime(timeArray))
        if timestamp < timestamp_9 or timestamp > timestamp_11:
            continue

        #其他数据必须要求有广告
        ad_top_num = items["ad_top_num"]
        ad_pzl_num = items["ad_pzl_num"]
        total_ad = ad_top_num + ad_pzl_num

        #统计9-11点的数据
        allQuery_in_9_11 += 1
        if ad_top_num > 0:
            ad2Query_in_9_11 += 1
        if total_ad > 0:
            ad1_ad2Query_in_9_11 += 1

        #省市信息
        prov_city=items["prov_city"]

        #看看是不是北上广的数据
        ct = ""
        if prov_city.find("-") > 0:
            ct = prov_city.split("-")
            ct = ct[1].rstrip(city_str)
        else:
            ct = prov_city.rstrip(city_str)
        if ct in city_list:
            bsg_Query_in_9_11 += 1
            if ad_top_num > 0:
                bsg_ad2Query_in_9_11 += 1
            if total_ad > 0:
                bsg_ad1_ad2Query_in_9_11 += 1

        #然后再统计其他数据
        if total_ad <= 0:
            continue
        query = items["query"]
        html_file = items["html_file"]
        #记录全量的数据
        if query not in all_query:
            all_query[query] = dict({
                "query":query,
                "time":items["time"],
                "prov_city":prov_city,
                "html_file":html_file,
                "ad_top_num":ad_top_num,
                "ad_pzl_num":ad_pzl_num,
                "list":items["list"][:]
            })

        if ct not in city_list:
            continue

        #先记录北上广的数据，再随机选取
        if query not in bsg_query:
            bsg_query[query] = dict({
                "query":query,
                "prov_city":ct,
                "html_file":html_file,
                "time":items["time"],
                "ad_top_num":ad_top_num,
                "ad_pzl_num":ad_pzl_num,
                "list":items["list"][:]
            })


    #打印广告统计数据
    print "allQuery_in_9_11(all PV in 09:00 ~ 11:00)=%d" % allQuery_in_9_11
    print "ad2Query_in_9_11(top ad num in 09:00 ~ 11:00)=%d" % ad2Query_in_9_11
    print "ad1_ad2Query_in_9_11(top ad and Pinpai ad num in 09:00 ~ 11:00)=%d" % ad1_ad2Query_in_9_11
    print "bsg_Query_in_9_11(BeiShangGuang PV num in 09:00 ~ 11:00)=%d" % bsg_Query_in_9_11
    print "bsg_ad2Query_in_9_11(BSG top ad num in 09:00 ~ 11:00)=%d" % bsg_ad2Query_in_9_11
    print "bsg_ad1_ad2Query_in_9_11(BSG top ad and Pinpai ad num in 09:00 ~ 11:00)=%d" % bsg_ad1_ad2Query_in_9_11


    #最后，开始随机选取
    for num in range(200):
        tmp = all_query.popitem()
        all200.append(tmp[1])
    for num in range(100):
        tmp = bsg_query.popitem()
        bsg100.append(tmp[1])

    del all_query
    del bsg_query
    return True

################################################################
# 这些数据文件导入到系统后，获取产生的任务名称
################################################################
def get_task_name( data_file ):
    ret = "autotask-%s" % os.path.basename(data_file)
    return ret[:-4]

################################################################
# 将文件数据写入文件，query,url,rank,engine,is_ad,prov_city 以\t分隔
################################################################
def write_ad_data(tm_str):
    global all200_with_ad_dict,all200_no_ad_dict,all200,bsg100,bsg100_with_ad_dict,bsg100_no_ad_dict
    #全量200词的文件
    all200_with_ad_file = all200_with_ad % tm_str
    all200_with_ad_file = "%s/%s" % (sogou_conf["stat_data_dir"].rstrip("/"),all200_with_ad_file)
    utils.mk_sure_dir(all200_with_ad_file)
    all200_no_ad_file = all200_no_ad % tm_str
    all200_no_ad_file = "%s/%s" % (sogou_conf["stat_data_dir"].rstrip("/"),all200_no_ad_file)
    utils.mk_sure_dir(all200_no_ad_file)
    for l in all200:
        ad_tmp = dict({
            "query":l["query"],
            "prov_city":l["prov_city"],
            "engine":"sogou",
            "html_file":l["html_file"],
            "time":l["time"],
            "list":list(),
        })
        no_tmp = dict({
            "query":l["query"],
            "prov_city":l["prov_city"],
            "engine":"sogou",
            "html_file":l["html_file"],
            "time":l["time"],
            "list":list(),
        })
        for item in l["list"]:
            if len(ad_tmp["list"]) < 3:
                ad_tmp["list"].append(copy.deepcopy(item))
            if item["ad_type"] == 0 and len(no_tmp["list"]) < 3:
                no_tmp["list"].append(copy.deepcopy(item))
        all200_with_ad_dict[l["query"]] = copy.deepcopy(ad_tmp)
        all200_no_ad_dict[l["query"]] = copy.deepcopy(no_tmp)
    fd = open(all200_with_ad_file,"w")
    for q in all200_with_ad_dict:
        rank = 1
        lists = all200_with_ad_dict[q]
        for l in lists["list"]:
            url = l["url"] if len(l["url"]) > 0 else l["raw_url"]
            is_ad = "1" if l["ad_type"] > 0 else "0"
            ostr = "%s\t%s\t%s\t%s\t%s\t%s\n" % (lists["query"].encode("gbk"),\
                     url.encode("gbk"),str(rank),lists["engine"],is_ad,lists["prov_city"].encode("gbk"))
            rank += 1
            fd.write(ostr)
            fd.flush()
    fd.close()
    #全量200不带广告的
    fd = open(all200_no_ad_file,"w")
    for q in all200_no_ad_dict:
        rank = 1
        lists = all200_no_ad_dict[q]
        for l in lists["list"]:
            url = l["url"] if len(l["url"]) > 0 else l["raw_url"]
            is_ad = "1" if l["ad_type"] > 0 else "0"
            ostr = "%s\t%s\t%s\t%s\t%s\t%s\n" % (lists["query"].encode("gbk"),\
                     url.encode("gbk"),str(rank),lists["engine"],is_ad,lists["prov_city"].encode("gbk"))
            rank += 1
            fd.write(ostr)
            fd.flush()
    fd.close()

    #北上广100词的处理
    bsg100_with_ad_file = bsg100_with_ad % tm_str
    bsg100_with_ad_file = "%s/%s" % (sogou_conf["stat_data_dir"].rstrip("/"),bsg100_with_ad_file)
    utils.mk_sure_dir(bsg100_with_ad_file)
    bsg100_no_ad_file = bsg100_no_ad % tm_str
    bsg100_no_ad_file = "%s/%s" % (sogou_conf["stat_data_dir"].rstrip("/"),bsg100_no_ad_file)
    utils.mk_sure_dir(bsg100_no_ad_file)
    for l in bsg100:
        ad_tmp = dict({
            "query":l["query"],
            "prov_city":l["prov_city"],
            "engine":"sogou",
            "html_file":l["html_file"],
            "time":l["time"],
            "list":list(),
        })
        no_tmp = dict({
            "query":l["query"],
            "prov_city":l["prov_city"],
            "engine":"sogou",
            "html_file":l["html_file"],
            "time":l["time"],
            "list":list(),
        })
        for item in l["list"]:
            if len(ad_tmp["list"]) < 3:
                ad_tmp["list"].append(copy.deepcopy(item))
            if item["ad_type"] == 0 and len(no_tmp["list"]) < 3:
                no_tmp["list"].append(copy.deepcopy(item))
        bsg100_with_ad_dict[l["query"]] = copy.deepcopy(ad_tmp)
        bsg100_no_ad_dict[l["query"]] = copy.deepcopy(no_tmp)
    #带广告的
    fd = open(bsg100_with_ad_file,"w")
    for q in bsg100_with_ad_dict:
        rank = 1
        lists = bsg100_with_ad_dict[q]
        for l in lists["list"]:
            url = l["url"] if len(l["url"]) > 0 else l["raw_url"]
            is_ad = "1" if l["ad_type"] > 0 else "0"
            ostr = "%s\t%s\t%s\t%s\t%s\t%s\n" % (lists["query"].encode("gbk"),\
                     url.encode("gbk"),str(rank),lists["engine"],is_ad,lists["prov_city"].encode("gbk"))
            rank += 1
            fd.write(ostr)
            fd.flush()
    fd.close()
    #不带广告的
    fd = open(bsg100_no_ad_file,"w")
    for q in bsg100_no_ad_dict:
        rank = 1
        item = bsg100_no_ad_dict[q]
        for l in item["list"]:
            url = l["url"] if len(l["url"]) > 0 else l["raw_url"]
            is_ad = "1" if l["ad_type"] > 0 else "0"
            ostr = "%s\t%s\t%s\t%s\t%s\t%s\n" % (item["query"].encode("gbk"),\
                     url.encode("gbk"),str(rank),item["engine"],is_ad,item["prov_city"].encode("gbk"))
            rank += 1
            fd.write(ostr)
            fd.flush()
    fd.close()

    return (all200_with_ad_file, all200_no_ad_file, bsg100_with_ad_file, bsg100_no_ad_file)

################################################################
# 将生成的文件传送到131机器上，并获取task_id用来拼凑snapshot_url
################################################################
def rsycn_data(tm_str):
    year_month = tm_str[0:4] + "-" + tm_str[4:6]
    snapshot_dir = "/search/oss/new/application/snapshot/%s/%s_1/"
    global task200_file,bsg100_file,rsync_cmd
    task200 = task200_file % tm_str
    task200 = "%s/%s" % (sogou_conf["stat_data_dir"].rstrip("/"),task200)
    bsg100 = bsg100_file % tm_str
    bsg100 = "%s/%s" % (sogou_conf["stat_data_dir"].rstrip("/"),bsg100)

    monitor_files = write_ad_data(tm_str)
    all200_with_ad_file = monitor_files[0]
    os.system(rsync_cmd % all200_with_ad_file)
    all200_no_ad_file = monitor_files[1]
    os.system(rsync_cmd % all200_no_ad_file)
    bsg100_with_ad_file = monitor_files[2]
    os.system(rsync_cmd % bsg100_with_ad_file)
    bsg100_no_ad_file = monitor_files[3]
    os.system(rsync_cmd % bsg100_no_ad_file)

    #等5分钟再去查结果
    time.sleep(360)
    db = DB(user="phpmyadmin",passwd="123456",host="10.11.195.131",db="mark_oss",port=3306,charset="utf8")
    all200_with_ad_task_name = get_task_name(all200_with_ad_file)
    all200_with_ad_task_id = db.fetch_col("SELECT `tid` FROM `tb_dcg_task` WHERE `name`='%s' "
                                          "ORDER BY `createtime` DESC LIMIT 1" % all200_with_ad_task_name)
    all200_no_ad_task_name = get_task_name(all200_no_ad_file)
    all200_no_ad_task_id = db.fetch_col("SELECT `tid` FROM `tb_dcg_task` WHERE `name`='%s' "
                                          "ORDER BY `createtime` DESC LIMIT 1" % all200_no_ad_task_name)
    bsg100_with_ad_task_name = get_task_name(bsg100_with_ad_file)
    bsg100_with_ad_task_id = db.fetch_col("SELECT `tid` FROM `tb_dcg_task` WHERE `name`='%s' "
                                          "ORDER BY `createtime` DESC LIMIT 1" % bsg100_with_ad_task_name)
    bsg100_no_ad_task_name = get_task_name(bsg100_no_ad_file)
    bsg100_no_ad_task_id = db.fetch_col("SELECT `tid` FROM `tb_dcg_task` WHERE `name`='%s' "
                                          "ORDER BY `createtime` DESC LIMIT 1" % bsg100_no_ad_task_name)

    #然后写入发给xueying的文件中,query,url,rank,engine,is_ad,prov_city,snapshot_url 以,分隔
    fd = open(task200,"w")
    for q in all200_with_ad_dict:
        #带广告的
        items = all200_with_ad_dict[q]
        rank = 1
        for list in items["list"]:
            surl = snapshot_url % (all200_with_ad_task_id,q)
            url = list["url"] if len(list["url"]) > 0 else list["raw_url"]
            ostr = "%s,\"%s\",\"%s\",%s,%s,%s,%s,%s,\"%s\"\n" % (
                items["time"], q.encode("gbk"),url.encode("gbk"),str(rank),"sogou",str(list["ad_type"]),
                items["prov_city"].encode("gbk"),"含广告".encode("gbk"),surl.encode("gbk")
            )
            fd.write(ostr)
            rank += 1
        html_file = items["html_file"]
        tmp_dir = snapshot_dir % (year_month,all200_with_ad_task_id)
        tmp_file = tmp_dir + hashlib.md5(q.encode("utf-8")).hexdigest() + ".html"
        os.system("(ssh root@10.11.195.131 mkdir -p %s ; rsync %s root@10.11.195.131:%s)" % (tmp_dir,html_file,tmp_file))
        #不带广告的
        items = all200_no_ad_dict[q]
        rank = 1
        for list in items["list"]:
            surl = snapshot_url % (all200_no_ad_task_id,q)
            url = list["url"] if len(list["url"]) > 0 else list["raw_url"]
            ostr = "%s,\"%s\",\"%s\",%s,%s,%s,%s,%s,\"%s\"\n" % (
                items["time"], q.encode("gbk"),url.encode("gbk"),str(rank),"sogou",str(list["ad_type"]),
                items["prov_city"].encode("gbk"),"除广告".encode("gbk"),surl.encode("gbk")
            )
            fd.write(ostr)
            rank += 1
        html_file = items["html_file"]
        tmp_dir = snapshot_dir % (year_month,all200_no_ad_task_id)
        tmp_file = tmp_dir + hashlib.md5(q.encode("utf-8")).hexdigest() + ".html"
        os.system("(ssh root@10.11.195.131 mkdir -p %s ; rsync %s root@10.11.195.131:%s)" % (tmp_dir,html_file,tmp_file))
    fd.close()

    fd = open(bsg100,"w")
    for q in bsg100_with_ad_dict:
        #带广告的
        items = bsg100_with_ad_dict[q]
        rank = 1
        for list in items["list"]:
            surl = snapshot_url % (bsg100_with_ad_task_id,q)
            url = list["url"] if len(list["url"]) > 0 else list["raw_url"]
            ostr = "%s,\"%s\",\"%s\",%s,%s,%s,%s,%s,\"%s\"\n" % (
                items["time"], q.encode("gbk"),url.encode("gbk"),str(rank),"sogou",str(list["ad_type"]),
                items["prov_city"].encode("gbk"),"含广告".encode("gbk"),surl.encode("gbk")
            )
            fd.write(ostr)
            rank += 1
        html_file = items["html_file"]
        tmp_dir = snapshot_dir % (year_month,bsg100_with_ad_task_id)
        tmp_file = tmp_dir + hashlib.md5(q.encode("utf-8")).hexdigest() + ".html"
        os.system("(ssh root@10.11.195.131 mkdir -p %s ; rsync %s root@10.11.195.131:%s)" % (tmp_dir,html_file,tmp_file))
        #不带广告的
        items = bsg100_no_ad_dict[q]
        rank = 1
        for list in items["list"]:
            surl = snapshot_url % (bsg100_no_ad_task_id,q)
            url = list["url"] if len(list["url"]) > 0 else list["raw_url"]
            ostr = "%s,\"%s\",\"%s\",%s,%s,%s,%s,%s,\"%s\"\n" % (
                items["time"], q.encode("gbk"),url.encode("gbk"),str(rank),"sogou",str(list["ad_type"]),
                items["prov_city"].encode("gbk"),"除广告".encode("gbk"),surl.encode("gbk")
            )
            fd.write(ostr)
            rank += 1
        html_file = items["html_file"]
        tmp_dir = snapshot_dir % (year_month,bsg100_no_ad_task_id)
        tmp_file = tmp_dir + hashlib.md5(q.encode("utf-8")).hexdigest() + ".html"
        os.system("(ssh root@10.11.195.131 mkdir -p %s ; rsync %s root@10.11.195.131:%s)" % (tmp_dir,html_file,tmp_file))
    fd.close()

    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "USAGE %s %s" % (sys.argv[0],"sogou_ad_file")
        sys.exit()
    ret_file=sys.argv[1]
    if not os.path.isfile(ret_file):
        print "file not exists %s" % ret_file
        sys.exit()
    tmp=os.path.dirname(ret_file)
    tm_str=os.path.basename(tmp)
    get_sogou_stat(ret_file)
    rsycn_data(tm_str)