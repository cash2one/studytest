#include "httpdh.h"
#include "gen_sign.h"
#include "utils.h"
#include<libxml/parser.h>
#include<libxml/tree.h>
#include<libxml/xmlschemas.h>
#include "mod_newpassport.h"

/**
 * 递归遍历SAML树
 */
static void parse_saml_recur(request_rec *r, xmlNodePtr curNode, saml_elem* se)
{
    xmlNodePtr tmpNode = NULL;
    tmpNode = curNode->xmlChildrenNode;
    char* nodeStr = NULL;

    while(NULL != tmpNode){
        if(xmlStrcmp(tmpNode->name, BAD_CAST "NameIdentifier") == 0){///提取用户名
            nodeStr = (char*) xmlNodeGetContent(tmpNode);
            se->username = (char*) apr_pstrdup(r->pool, nodeStr);
            xmlFree(BAD_CAST nodeStr);
        }
        else if(xmlStrcmp(tmpNode->name, BAD_CAST "Conditions") == 0){//提取最早的开始时间及最晚的结束时间
            nodeStr = (char*) xmlGetProp(tmpNode, BAD_CAST "NotBefore");
            se->notbefore = (char*) apr_pstrdup(r->pool, nodeStr);
            xmlFree(BAD_CAST nodeStr);
            nodeStr = (char*) xmlGetProp(tmpNode, BAD_CAST "NotOnOrAfter");
            se->notafter = (char*) apr_pstrdup(r->pool, nodeStr);
            xmlFree(BAD_CAST nodeStr);
        }
        else if(xmlStrcmp(tmpNode->name, BAD_CAST "SignatureValue") == 0){///提取SignedInfo部分文档的签名字段值
            nodeStr = (char*) xmlNodeGetContent(tmpNode);
            se->SignatureValue = (char*) apr_pstrdup(r->pool, nodeStr);
            remove_lineends(se->SignatureValue);
            xmlFree(BAD_CAST nodeStr);
        }
        else if(xmlStrcmp(tmpNode->name, BAD_CAST "DigestValue") == 0){///提取SAML有效文档的摘要值
            nodeStr = (char*) xmlNodeGetContent(tmpNode);
            se->DigestValue = (char*) apr_pstrdup(r->pool, nodeStr);
            remove_lineends(se->DigestValue);
            xmlFree(BAD_CAST nodeStr);
        }
        parse_saml_recur(r, tmpNode, se);
        tmpNode = tmpNode->next;
    }
}

/**
 * 利用saml的schema、signature验证所得到的SAML文档是否合法
 */
int verify_saml_response(request_rec* r, const char* responsexml)
{
    return 0;
}

/**
 * 遍历SAML树，提取出下面几个节点里的信息，然后再利用privkeyaddr、password计算签名
 * 
 * NameIdentifier节点里的用户名
 * Conditions节点里的NotBefore、NotOnOrAfter属性
 */
int get_saml_elem_value(request_rec *r, saml_elem* se)
{
    xmlDocPtr samlDoc = NULL;
    xmlNodePtr rootNode = NULL;

    //构造SAML结构树
    samlDoc = xmlParseDoc(se->saml_str);
    if(NULL == samlDoc){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] parse XML failed\n");
        return -1;
    }

    //获取根节点
    rootNode = xmlDocGetRootElement(samlDoc);
    if(NULL == rootNode){
        xmlFreeDoc(samlDoc);
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get root element failed\n");
        return -2;
    }

    //获取username,notbefore,notafter三个值
    parse_saml_recur(r, rootNode, se);
    xmlFreeDoc(samlDoc);
    if(NULL == se->username || NULL == se->notbefore || se->notafter == NULL){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get username/notbefore/notafter failed\n");
        return -4;
    }

    ///计算签名值
    char* strsum = (char*) apr_pstrcat(r->pool, se->username, se->notbefore, se->notafter, NULL);
    int ret = sign(r, strsum, &(se->sign));
    if(ret != 0){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get sign failed\n");
        return -5;
    }

    return 0;
}