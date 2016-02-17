#include<iostream>
#include "stdfx.h"
#include<map>
#include<string>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<memory>

using namespace std;

class test{
public:
    test(){
        cout<<"test::test()"<<endl;
    }
    int a;
    ~test(){
        cout<<"test::~test()"<<endl;
    }
    void operator()(int* p){
        if(NULL != p){
           cout<<"release"<<endl;
           free(p);
        }
    }
};

void release(int* p){
    if(NULL != p){
        cout<<"release"<<endl;
        free(p);
    }
}

int main(int argc, char *argv[]){
    //char *str = "<wendao><name>liuyongshuai</name><age>28</age><company>sogou</company></wendao>";
    //test_string test(str);
    //test.run();

    //test_map tmap;
    //tmap.run();

    //test_vector tvec;
    //tvec.run();

    //test_list tlist;
    //tlist.run();

    //test_set tset;
    //tset.run();

    //test_stack stk;
    //stk.run();

    //test_queue tq;
    //tq.run();

    //test_pair tpr;
    //tpr.run();

    /*
    const char* ptr = getStr(10,'a');
    printf("%s\n",ptr);
    const int* pint = getInt(3);
    printf("%d\n",*pint);
    const char* pos = getPos();
    printf("%s\t%c\n",pos,*pos);
    char* p1 = getCharArr(5);
    printf("%s\n",p1);
    */


    /*
    test_class test1;
    test1.show();
    test1.showAll();
    test_class::show();

    test_class test2 = test1;

    test_class test3(99);
    test_class::show();
    test3.showAll();

    test3 = test2;
    test3.showAll();
    int i = (int)test3;
    printf("%d\n",i);

    test1 += test3;
    test1.showAll();
    test1.printStr();

    test_class *test_ptr = new test_class(100);
    test_ptr->show();
    test_ptr->printStr();

    test_class_child *test_child = new test_class_child(52,21);
    test_child->showAll();
    test_child->printStr();

    test_ptr->num = 2000;
    test_child->num = 6000;

    printf("+++++++++++++++++++++++++++++++++++++++\n");
    test_ptr = (test_class *)test_child;
    try{
        test_ptr->printTest();
    }
    catch(map<string,string> &e){
        cout<<"receive map<string,string> type exception"<<endl;
        map<string,string>::iterator iter;
        for(iter=e.begin();iter!=e.end();iter++){
            cout<<iter->first<<"\t"<<iter->second<<endl;
        }
    }
    test_ptr->printStr();
    printf("%d\n",test_ptr->num);
    */


    /*
    struct test_struct *ptr = (struct test_struct*)malloc(sizeof(struct test_struct));
    char *p1 = "liuyongshuai";
    char *p2 = "tencent";
    char *p3 = "wendao";
    ptr->num[0] = strlen(p1);
    strcpy(ptr->data,p1);
    ptr->num[1] = strlen(p2);
    strcpy(ptr->data+strlen(p1),p2);
    ptr->num[2] = strlen(p3);
    strcpy(ptr->data+strlen(p1)+strlen(p2),p3);

    int i;
    char buf[20];
    char *st = ptr->data;
    for(i=0; i<3; i++){
        bzero(buf,20);
        strncpy(buf,st,ptr->num[i]);
        st +=  ptr->num[i];
        printf("%s\n",buf);
    }
    */

    //test_algorithm ta;
    //ta.run();

    //test_ext_hash_map tehm;
    //tehm.run();

    //test_ext_hash_set tehs;
    //tehs.run();

    //test_ext_hash_slist telt;
    //telt.run();

    /*
    shared_ptr<int> p0(new int(100));
    int* p = new int(10);
    {
        shared_ptr<int> p1(p);
        shared_ptr<int> p2(p);
    }
    *p = 59;
    cout<<p<<"\t"<<*p<<endl;
    shared_ptr<int> abc = make_shared<int>(20);
    cout<<abc<<"\t"<<*abc<<endl;
    */


    /*
    int* p = new int(78);
    test* pt = new test;
    int* pa = (int*)malloc(sizeof(int));
    *pa = 999;
    pt->a = 44;
    {
        unique_ptr<int> p1(p);
        unique_ptr<test> p2(pt);
        unique_ptr<int> p3(pa);
    }
    cout<<*p<<endl;
    cout<<pt->a<<endl;
    cout<<*pa<<endl;
    */

    //test_iterator tit;
    //tit.run();

    //test_deque tdq;
    //tdq.run();

/*
    MD5 md5;  
    md5.update("阿尔法·罗密欧");  
    fprintf(stderr,"%s\n",md5.toString().c_str());

    md5.reset();
    md5.update("周秀娜");  
    fprintf(stderr,"%s\n",md5.toString().c_str());
*/

/*
    DB db;
    db.initDB();
	///测试一个字段
    string sql = "select url from tb_engine where eid=?";
    vector<string> param;
    param.push_back("42");
    string result;
    db.fetch_one(sql,param,result);
	cout<<result<<endl;
	
	///测试一条数据
	string sql1 = "select * from tb_engine where eid=?";
    vector<string> param1;
    param1.push_back("42");
    map<string,string> result1;
	int ret = db.fetch_row(sql1,param1,result1);
    cout<<"db.fetch_row ret="<<ret<<endl;
	map<string,string>::iterator iter;
	cout<<endl;
	cout<<endl;
	for(iter = result1.begin(); iter != result1.end(); iter++){
		cout<<iter->first<<"\t"<<iter->second<<endl;
	}
	
	///测试返回所有结果的
	string sql2 = "select * from experiment_detail";
	vector<string> param2;
	map<int,map<string,string>> result2;
	db.fetch_all(sql2,param2,result2);
	map<int,map<string,string>>::iterator iter2;
	map<string,string>::iterator iter21;
	cout<<endl;
	cout<<endl;
	for(iter2=result2.begin(); iter2 != result2.end(); iter2++){
		cout<<"================row: "<<iter2->first<<endl;
		for(iter21=iter2->second.begin(); iter21!=iter2->second.end(); iter21++){
			cout<<"field="<<iter21->first<<"\tvalue="<<iter21->second<<endl;
		}
	}
    
	///测试一条INSERT语句
	map<string,string> param_insert;
	param_insert["exp_id"] = "8888";
	param_insert["module_name"] = "10";
	param_insert["para_name"] = "测试一条数据";
	param_insert["para_value"] = "这是直接插入的";
	param_insert["para_desc"] = "描述信息";
	cout<<endl;
	cout<<endl;
	int last_insert_id = db.insert("experiment_detail_para", param_insert);
	cout<<"last_insert_id="<<last_insert_id<<endl;
	
	
	///测试一条UPDATE语句
	map<string,string> update_data;
	update_data["para_desc"] = "ABCEDDDDDD";
	vector<string> update_arg;
	update_arg.push_back("100");
	update_arg.push_back("101");
	map<string,vector<string>> update_where;
	update_where["detail_para_id:in"] = update_arg;
	db.update("experiment_detail_para",update_data,update_where);
	
	///第二条UPDATE语句
	map<string,string> update_data2;
	update_data2["para_desc"] = "ASDFASDFSADFASD";
	vector<string> update_arg2;
	update_arg2.push_back("100");
	map<string,vector<string>> update_where2;
	update_where2["detail_para_id:gt"] = update_arg2;
	db.update("experiment_detail_para",update_data2,update_where2);
*/

/*
    mark_oss db;
    db.initDB();
	{
		map<string,vector<string>> where;
		vector<string> v;
		v.push_back("1000000");
		where["tid"] = v;
		int total = db.getTotal("tb_dcg_query",where);
		cout<<"total="<<total<<endl;
	}
	
	{
		map<string,vector<string>> where;
		vector<string> v;
		v.push_back("1000000");
		where["tid"] = v;
		map<string, string> orderby;
		orderby["seltime"] = "DESC";
		map<string, map<string, string>> ret;
		db.getList("tb_dcg_query",where,1,200,"query",orderby,ret);
		map<string, map<string, string>>::iterator outer;
		map<string, string>::iterator inner;
		for(outer=ret.begin(); outer!=ret.end(); outer++){
			cout<<"format_field_value="<<outer->first<<endl;
			for(inner=outer->second.begin(); inner!=outer->second.end(); inner++){
				cout<<inner->first<<"="<<inner->second<<"\t";
			}
			cout<<endl;
			cout<<endl;
		}
	}
*/
/*
    ConfigParser conf("./config.ini");	
    string value;
    conf.get_item("MYSQL_MARK_OSS_HOST",value);
    cout<<value<<endl;
*/

/*
	vector<string> xpath;
	xpath.push_back("//div[@id=1]/h3[@class=t]/a[@href]");
	xpath.push_back("//div[@id=1]/div[@class=op_url_size]");
	parseHTML t;
	t.setXpath(xpath);
	t.setPage("./fffb581a8b9947f2ba57c6edd555be3c.html");
	map<string, map<string, string> > ret;
	t.parse(ret);
*/
	test_regex();

    return 0;
}





