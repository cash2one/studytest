#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include "mysql.h"
#include "utils/utils.h"
#include "send_so/send_so.h"


char* const host = "10.11.195.131";
int port = 3306;
char* const user = "phpmyadmin";
char* const passwd = "123456";
char* const db = "mark_oss";


int main(){
//    hello_world();
//    send_so_print();
    
    MYSQL mysql;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,host,user,passwd,db,0,NULL,0)){
        fprintf(stderr,"connect MySQL failed:%s\n",mysql_error(&mysql));
        exit(EXIT_FAILURE);
    }
/*
    //all data in experiment_detail
    char* sql = "select * from `experiment_detail`";
    if(mysql_real_query(&mysql,sql,strlen(sql)) != 0){
        fprintf(stderr,"%s\n",mysql_error(&mysql));
        exit(EXIT_FAILURE);
    }
    MYSQL_RES *list = mysql_store_result(&mysql);
    my_ulonglong field_num = mysql_field_count(&mysql);
    my_ulonglong num_rows = mysql_num_rows(list);
    fprintf(stderr,"mysql_field_count()=%lu\n",field_num);
    fprintf(stderr,"mysql_num_rows()=%lu\n",num_rows);

    MYSQL_FIELD* field_list = mysql_fetch_fields(list);
    MYSQL_ROW row;
    int i = 0, j = 0;
    for(;i<field_num;i++){
        fprintf(stderr,"field:%d=%s\n",i,field_list[i].name);
    }
    fprintf(stderr,"\n\n\n");
    for(j=0;j<num_rows;j++){
        row = mysql_fetch_row(list);
        for(i=0;i<field_num;i++){
            fprintf(stderr,"%s=%s\n",field_list[i].name,row[i]);
        }
        fprintf(stderr,"\n\n\n");
    }
    mysql_free_result(list);
*/

    //prepared statements test
    MYSQL_STMT *stmt = mysql_stmt_init(&mysql);
    if(!stmt){
        fprintf(stderr,"mysql_stmt_init failed");
        exit(EXIT_FAILURE);
    }
    char* sql_str = "SELECT `flow_group` FROM `experiment_detail` WHERE `exp_id`<? OR `exp_name`=?";
    if(mysql_stmt_prepare(stmt,sql_str,strlen(sql_str) ) !=0){
        fprintf(stderr,"prepare sql failed:%s\n",mysql_stmt_error(stmt));
        fprintf(stderr,"errno=%d\n",mysql_stmt_errno(stmt));
        fprintf(stderr,"SQL=%s\n",sql_str);
        exit(EXIT_FAILURE);
    }
    unsigned long para_num = mysql_stmt_param_count(stmt);
    fprintf(stderr,"param_count=%lu\n",para_num);
    MYSQL_RES* meta_data = mysql_stmt_result_metadata(stmt);
    int column_num = mysql_num_fields(meta_data);
    fprintf(stderr,"column_num=%d\n",column_num);
    MYSQL_BIND bind[para_num];
    bzero(bind,sizeof(bind));
    
    int exp_id = 4;
    char exp_name[] = "sdds";
    unsigned long name_len = strlen(exp_name);
    //bind parameters
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char*)&exp_id; 
    bind[0].is_null = 0;
    bind[0].length = 0;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)exp_name;
    bind[1].is_null = 0;
    bind[1].length = &name_len;
    if(mysql_stmt_bind_param(stmt,bind) != 0){
        fprintf(stderr,"mysql_stmt_bind_param failed:%s\n",mysql_stmt_error(stmt));
        exit(EXIT_FAILURE);
    }
    if(mysql_stmt_execute(stmt)!=0){
        fprintf(stderr,"execute failed:%s\n",mysql_stmt_error(stmt));
        exit(EXIT_FAILURE);
    }


    MYSQL_BIND retBind[1];
    bzero(retBind,sizeof(retBind));
    unsigned long ret_len = 0;
    my_bool is_null = 0;
    char data[200] = {0};
    retBind[0].buffer = 0;
    retBind[0].buffer_length = 0;
    retBind[0].length = &ret_len;
    retBind[0].buffer_type = MYSQL_TYPE_VAR_STRING;
    retBind[0].is_null = (my_bool*)&is_null;
    if(mysql_stmt_bind_result(stmt, retBind) != 0){
        fprintf(stderr,"mysql_Stmt_bind_result failed: %s\n",mysql_stmt_error(stmt));
        exit(EXIT_FAILURE);
    }
    if(0 != mysql_stmt_store_result(stmt)){
        fprintf(stderr,"store_result failed: %s\n",mysql_stmt_error(stmt));
        exit(EXIT_FAILURE);
    }
    my_ulonglong num_rows = mysql_stmt_num_rows(stmt);
    fprintf(stderr,"num_rows=%lu\n",num_rows);
    int m = 0;
    for(;m<num_rows;m++){
        if(!mysql_stmt_fetch(stmt)){
            break;
        }
        retBind[0].buffer = data;
        retBind[0].buffer_length = ret_len;
        if(0!=mysql_stmt_fetch_column(stmt,retBind,0,0)){
            fprintf(stderr,"%s\n",mysql_stmt_error(stmt));
            exit(EXIT_FAILURE);
        }
        fprintf(stderr,"ret_len=%lu\tdata=%s\n",ret_len,data);
    }
    return 1;
}
