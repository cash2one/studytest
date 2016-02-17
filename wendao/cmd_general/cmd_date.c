#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>


static char show_cal = 'n';

#define Y_FLAG 0b00000001
#define M_FLAG 0b00000010
#define D_FLAG 0b00000100
#define H_FLAG 0b00001000
#define I_FLAG 0b00010000
#define S_FLAG 0b00100000
#define C_FLAG 0b01000000
#define T_FLAG 0b10000000

char option = 0;
static void showCal(const struct tm *stm,const time_t timestamp);

//无参数时显示当前日期
//-y显示4位年份
//-m显示2位月份
//-d显示当前日期
//-h显示小时
//-i显示分钟
//-s显示秒数
//-c显示本月日历，如cal命令
//-t后接时间戳，显示为日期形式
int cmd_date(int argc,char *argv[]){
    int ch;
    long time_stamp=time(NULL);
    while((ch=getopt(argc,argv,"ymdhisct:")) != -1){
        switch(ch){
            case 'y':option |= Y_FLAG;break;
            case 'm':option |= M_FLAG;break;
            case 'd':option |= D_FLAG;break;
            case 'h':option |= H_FLAG;break;
            case 'i':option |= I_FLAG;break;
            case 's':option |= S_FLAG;break;
            case 'c':option |= C_FLAG;break;
            case 't':time_stamp = atol(optarg);break;
        }
    }

    char buf[50]={0};
    if(option & Y_FLAG){
        strcat(buf,"%Y-");
    }
    if(option & M_FLAG){
        strcat(buf,"%m-");
    }
    if(option & D_FLAG){
        strcat(buf,"%d ");
    }
    if(option & H_FLAG){
        strcat(buf,"%H:");
    }
    if(option & I_FLAG){
        strcat(buf,"%M:");
    }
    if(option & S_FLAG){
        strcat(buf,"%S");
    }
    trim(buf,'-');
    trim(buf,':');
    trim(buf,32);

    struct tm *stm = localtime(&time_stamp);
    char fmt_buf[50];
    strftime(fmt_buf,50,buf,stm);
    printf("%s\n",fmt_buf);
    if(option & C_FLAG){
        printf("----------------calender--------------\n");
        showCal(stm,time_stamp);
    }
    return 0;
}


static void showCal(const struct tm *stm,const time_t timestamp){
    char m_y[20]={0};
    strftime(m_y,20,"%A %Y",stm);
    printf("\t\t%s\n",m_y);

    //获取每天的星期简写形式，如Sun
    char week_str[7][10]={0};
    int i=0;
    struct tm *tmp_tm;
    time_t tmp_time;
    char week_day[2]={0};
    char tmp_week[10]={0};
    for(; i<7; i++){
        bzero(tmp_week,10);
        tmp_time = timestamp + i * 86400;
        tmp_tm = localtime(&tmp_time);
        strftime(week_day,2,"%w",tmp_tm);
        strftime(tmp_week,10,"%a",tmp_tm);
        strcpy(week_str[atoi(week_day)],tmp_week);
    }
    for(i=0; i<7; i++){
        printf("%s\t",week_str[i]);
    }
    printf("\n");

    //获取当月的天数:下月月初-当月月初
    struct tm cmon_start;
    cmon_start.tm_year = stm->tm_year;
    cmon_start.tm_mon = stm->tm_mon;
    cmon_start.tm_mday = 1;
    cmon_start.tm_hour = 0;
    cmon_start.tm_min = 0;
    cmon_start.tm_sec = 0;
    time_t cur_mon = mktime(&cmon_start);

    struct tm nmon_start;
    nmon_start.tm_year = stm->tm_year;
    nmon_start.tm_mon = stm->tm_mon + 1;
    nmon_start.tm_mday = 1;
    nmon_start.tm_hour = 0;
    nmon_start.tm_min = 0;
    nmon_start.tm_sec = 0;
    time_t next_mon = mktime(&nmon_start);

    //当月的天数
    int mday_num = (int)((next_mon - cur_mon)/86400);

    //开始输出
    bzero(week_day,2);
    strftime(week_day,2,"%w",&cmon_start);
    int start_wday = atoi(week_day);
    int tmp_wday;
    time_t tmp_stamp;
    for(i=0; i<start_wday; i++){
        printf("\t");
    }
    for(i=0; i<mday_num; i++){
        printf("%d\t",i+1);
        bzero(week_day,2);
        tmp_stamp = cur_mon + i * 86400;
        tmp_tm = localtime(&tmp_stamp);
        strftime(week_day,2,"%w",tmp_tm);
        tmp_wday = atoi(week_day);
        if(6 == tmp_wday){
            printf("\n");
        }
    }
    printf("\n");
}
