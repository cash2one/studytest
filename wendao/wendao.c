#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include "stdfx.h"

//ls pwd cp sort uniq date rmdir split umask
//chmod chown wc join du df kill wget curl help
char *cmd[]={
	"ls","pwd","cp","sort","uniq","date","rmdir",
    "split","umask","chmod","chown","wc","wget","curl",
    "grep","find","help"
};
int cmdNum = 17;

void printUsage();
int checkCmd(const char *cmd);

//opt  不带值的选项
//opt: 必须带值
//opt:: 值是可选的
void main(int argc,char *argv[]){
	if(argc < 2){
		printUsage();
		exit(EXIT_FAILURE);
	}

	char *cmdStr = argv[1];
	if(checkCmd(cmdStr) != 0){
		printf("invalid command:%s\n",cmdStr);
		printf("try wendao help to get more information!\n");
		exit(EXIT_FAILURE);
	}
	if(strcmp(cmdStr,"help") == 0){
		printUsage();
		exit(EXIT_SUCCESS);
	}

    //命令分发
    if(strcmp(cmdStr,"ls") == 0){
        cmd_ls(argc,argv);
    }
    else if(strcmp(cmdStr,"pwd") == 0){
        cmd_pwd(argc,argv);
    }
    else if(strcmp(cmdStr,"cp") == 0){
        cmd_cp(argc,argv);
    }
    else if(strcmp(cmdStr,"sort") == 0){
        cmd_sort(argc,argv);
    }
    else if(strcmp(cmdStr,"uniq") == 0){
        cmd_uniq(argc,argv);
    }
    else if(strcmp(cmdStr,"date") == 0){
        cmd_date(argc,argv);
    }
    else if(strcmp(cmdStr,"rmdir") == 0){
        cmd_rmdir(argc,argv);
    }
    else if(strcmp(cmdStr,"split") == 0){
        cmd_split(argc,argv);
    }
    else if(strcmp(cmdStr,"umask") == 0){
        cmd_umask(argc,argv);
    }
    else if(strcmp(cmdStr,"chmod") == 0){
        cmd_chmod(argc,argv);
    }
    else if(strcmp(cmdStr,"chown") == 0){
        cmd_chown(argc,argv);
    }
    else if(strcmp(cmdStr,"wc") == 0){
        cmd_wc(argc,argv);
    }
    else if(strcmp(cmdStr,"wget") == 0){
        cmd_wget(argc,argv);
    }
}

void printUsage(){
	int num;
	printf("Usage : wendao <command> <optArgs>\n");
	printf("-command list:\n");
	for(num=0;num<cmdNum;num++){
		printf(" *%s\n",cmd[num]);
	}
}

int checkCmd(const char *cmdStr){
	int num;
	for(num=0; num < cmdNum; num++){
		if(strcmp(cmdStr,cmd[num]) == 0){
			return 0;
		}
	}
	return 1;
}
