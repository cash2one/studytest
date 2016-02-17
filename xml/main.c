#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <httpd.h>
#include <apr_strings.h>
#include <apr_strmatch.h>
#include <apr_general.h>
#include <apr_pools.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#define XML_FILE "/home/liuyongshuai/keys.xml"
#define CER_FILE "/home/liuyongshuai/public.cer"
#define KEY_FILE "/home/liuyongshuai/public.key"


void printNode(xmlNodePtr curNode){
    xmlNodePtr tmp = curNode->xmlChildrenNode;
    while(tmp != NULL){
        printf("\t\t\t\tnode->name : %s\n", tmp->name);
        printf("\t\t\t\tnode->content : %s\n", xmlNodeGetContent(tmp));
        printNode(tmp);
        tmp=tmp->next;
    }
}

void remove_lineends( char* buf ){
    char* p1 = buf;
    char* p2 = buf;
    if(buf == NULL) return;
    while(1){
        while(*p2 == '\r' || *p2 == '\n')
            p2++;
        *p1 = *p2;
        p1++;
        p2++;
        if(*p2 == '\0') break;
    }
    *p1 = '\0';
}

int main(int argc, char* argv[]){
    apr_status_t rv;
    apr_pool_t *pool;
    rv = apr_initialize();
    if(rv != APR_SUCCESS){
        fprintf(stderr,"apr_initialize failed\n");
        return -1;
    }
    apr_pool_create(&pool,NULL);


    //get base64 xml contents
    FILE* fp = fopen(XML_FILE,"r");
    char decode_buf[10240] = {0};
    size_t cursor = 0,by = 0;
    while(!feof(fp)){
        by = fread(decode_buf+cursor,1024,1,fp);
        cursor += by;
    }
    fclose(fp);


    //base64 decode
    remove_lineends(decode_buf);
    char* xml_str = ap_pbase64decode(pool, decode_buf);
fprintf(stderr,"xml_str=\n%s\n",xml_str);

    return 0;

}
