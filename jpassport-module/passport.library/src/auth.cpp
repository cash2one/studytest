#include <saml/saml.h> // for opensaml

#include <xercesc/util/XMLString.hpp>

#include <apr_base64.h>
#include <apr_general.h>
#include <apr_pools.h>

#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include <stdarg.h>   
#include <errno.h>   
#include <fcntl.h>   
#include <unistd.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include <netinet/in.h>   
#include <arpa/inet.h>   
  
#include <netdb.h>   

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>

#include <openssl/crypto.h>   
#include <openssl/ssl.h>   
#include <openssl/err.h>   
#include <openssl/rand.h>

#include <time.h>
#include <string>
#include <sstream>
#include <string>

using namespace saml;
using namespace std;

#define BUF_LEN        1024
#define MAX_STRING_LEN 2048
#define READ_STATUS    0
#define WRITE_STATUS   1
#define EXCPT_STATUS   2 

static int bIsHttps = 1;
static int timeout_sec = 10;
static int timeout_microsec = 0;

extern "C" void 
err_doit(int errnoflag, const char *fmt, va_list ap)
{
    int errno_save;
    char buf[MAX_STRING_LEN];   
  
    errno_save = errno;    
  
    vsprintf(buf, fmt, ap);   
  
    if (errnoflag)   
        sprintf(buf + strlen(buf), ": %s", strerror(errno_save));   
  
    strcat(buf, "\n");   
  
    fflush(stdout);   
    fputs(buf, stderr);   
    fflush(NULL);   
  
    return;   
  
}   

/**
 * 打印一条信息然后退出程序 
 */
extern "C" void 
err_quit(const char *fmt, ...)
{
    va_list ap;   
    va_start(ap, fmt);   
    err_doit(0, fmt, ap);   
    va_end(ap);   

    exit(1);   
}   

/**
 * 创建TCP Socket 连接 
 */
extern "C" int 
create_tcpsocket(const char *host, const unsigned short port)
{   
    int ret;   
    char * transport = "tcp";   
    struct hostent *phe; /* pointer to host information entry */  
    struct protoent *ppe; /* pointer to protocol information entry */  
    struct sockaddr_in sin; /* an Internet endpoint address */  
  
    int s; /* socket descriptor and socket type */
   
    memset(&sin, 0, sizeof(sin));   
    sin.sin_family = AF_INET;   
  
    if ((sin.sin_port = htons(port)) == 0)   
       err_quit("invalid port \"%d\"\n", port);   
 
    /* Map host name to IP address, allowing for dotted decimal */  
    if( phe = gethostbyname(host) )   
         memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);   
  
    else if( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )   
        err_quit("can't get \"%s\" host entry\n", host);   
     
    /* Map transport protocol name to protocol number */  
    if ( (ppe = getprotobyname(transport)) == 0)   
        err_quit("can't get \"%s\" protocol entry\n", transport);         
  
    /* Allocate a common TCP socket */  
    s = socket(PF_INET, SOCK_STREAM, ppe->p_proto);   
  
    if (s < 0)   
        err_quit("can't create socket: %s\n", strerror(errno));   
    
    /* create common tcp socket.seems non-block type is not supported by ssl. */
    ret = connect(s, (struct sockaddr *)&sin, sizeof(sin));

    if(ret != 0)
    {  
        close(s);
        err_quit("can't connect to %s:%d\n", host, port);
    }
  
    return s;   
}   

/**
 * 注意，返回的指针需要被释放
 */
extern "C" char*
saml_request_id_generator(void)
{
   char* id = NULL;
   srand(time(NULL));

   id = new char[32];

   for(int i = 0; i < 32; i++)
   {
      id[i] = 97 + rand() % 25;
   }

   return id;
}


/**
 * 由username、resource、action构建SAMLRequest对象
 */ 
extern "C" SAMLRequest* 
make_auth_decision_request(const char* name_identifier, const char* resource, const char* action)
{
    int rv;
    SAMLRequest* prequest = new SAMLRequest;

    SAMLNameIdentifier* pnameidentifier    = NULL;
    SAMLSubject*        psubject           = NULL;
    SAMLAction*         paction            = NULL;
    SAMLDateTime*       pdatetime          = NULL;
    SAMLAuthorizationDecisionQuery* pquery = NULL;
    time_t t;

    try
    {
        pnameidentifier  = new  SAMLNameIdentifier;
        XMLCh* xmlnameidentifier = XMLString::transcode(name_identifier);
       
        pnameidentifier->setName(xmlnameidentifier);
	
        psubject = new SAMLSubject;
        psubject->setNameIdentifier(pnameidentifier);

        paction = new SAMLAction;
        XMLCh* xmlAction = XMLString::transcode(action);
        paction->setData(xmlAction);

        pquery = new SAMLAuthorizationDecisionQuery;
        pquery->setSubject(psubject);
        // 将char*转为XMLCh*
        XMLCh* xmlResource = XMLString::transcode(resource); 
        pquery->setResource(xmlResource);
		pquery->addAction(paction);
        // 每一个request对象必须有一个唯一的ID
        char id[33];
        //sprintf(id, "%d",(int)time(&t)); // 用时间位数不足32位
        sprintf(id, "%s","ocmkpmcmmjfcjjbnmongfiaimldaljkn");
        XMLCh* xmlId = XMLString::transcode(id);
        prequest->setId(xmlId);

        // 为resquest对象设置时间戳
        pdatetime = new SAMLDateTime( time(&t) );
        prequest->setIssueInstant(pdatetime);

        // 次要版本??? 等于1???
        //prequest->setMinorVersion(minorVersion);
         
        // action和resource已经被加入到query中，
        // 再把query加入request对象中
        prequest->setQuery(pquery);

        XMLString::release(&xmlResource);
        XMLString::release(&xmlId);
        XMLString::release(&xmlnameidentifier);
        XMLString::release(&xmlAction);
        rv = 0;
    }
    catch(SAMLException& e)
    {
       cerr << "caught a SAML exception: " << e << endl;
       rv = -1;
    }

   // 何时清除这些组件是个大问题！！
   // delete pnameidentifier;
   // delete pquery;
   // delete paction;
   // delete pdatetime;

    if(rv != 0) return NULL;

    return prequest;
}

extern "C" void 
drop(char* buf)
{
    char* p1 = buf;
    char* p2 = buf;

    if(buf == NULL) return;

    while(1)
    {
        while(*p2 == '\r' || *p2 == '\n')
            p2++;
        *p1 = *p2;
        p1++;
        p2++;
        if(*p2 == '\0') break;
    }
    *p1='\0';
}

/**
 * 对URL中出现的特殊字符进行URL Encoding
 */
extern "C" string*
replace_lineends(char* str)
{
    string* strtemp = new string(str);

    string::size_type loc = strtemp->find("\n", 0);
    while(loc != string::npos)
    {
        strtemp->replace(loc, 1, "%0A");
        loc = strtemp->find("\n", 0);
    }

    loc = strtemp->find("\r", 0);
    while(loc != string::npos)
    {
        strtemp->replace(loc, 1, "%0D");
        loc = strtemp->find("\r", 0);
    }

    loc = strtemp->find("=", 0);
    while(loc != string::npos)
    {
        strtemp->replace(loc, 1, "%3D");
        loc = strtemp->find("=", 0);
    }

    loc = strtemp->find("+", 0);
    while(loc != string::npos)
    {
        strtemp->replace(loc, 1, "%2B");
        loc = strtemp->find("+", 0);
    }

    return strtemp;
} 

extern "C" char*
get_auth_decision_response(SAMLRequest* prequest, char* IDPaddress, char* IDPpath, unsigned int port)
{
    int ret;
    int fd; ////
    unsigned int requestbase64len = 0;
    char* request_buf = NULL;

    char header[10240];
    char response_buf[51200];
    response_buf[0] = '\0';

    // 传送的是Base64编码的内容
    request_buf = (char*)prequest->toBase64(&requestbase64len);
 
    //// 利用SSL实现HTTPS传输
    SSL *ssl;
    SSL_CTX *ctx;

    SSL_load_error_strings();
    SSL_library_init();
  
    ctx = SSL_CTX_new(SSLv23_client_method());

    if(ctx == NULL)
    {
        err_quit("init SSL CTX failed:%s\n",ERR_reason_error_string(ERR_get_error()));
    }

    ssl = SSL_new(ctx);
  
    if (ssl == NULL)
    {
        err_quit("new SSL with created CTX failed:%s\n",ERR_reason_error_string(ERR_get_error()));
    }   

    fd = create_tcpsocket(IDPaddress, port);
    ret = SSL_set_fd(ssl, fd);

    if (ret == 0)
    {
       err_quit("add SSL to tcp socket failed:%s\n", ERR_reason_error_string(ERR_get_error()));
    }
 
    RAND_poll();
    while (RAND_status() == 0)
    {
       unsigned short rand_ret = rand() % 65536;
       RAND_seed(&rand_ret, sizeof(rand_ret));
    }
  
    // 建立SSL连接
    ret = SSL_connect(ssl); 
    if(ret != 1)
    {
       err_quit("SSL connection failed:%s\n",ERR_reason_error_string(ERR_get_error()));
    }

    drop(request_buf);
    string* pstr = replace_lineends(request_buf);

    sprintf(header,"GET /%s?samlRequest=%s HTTP/1.1\r\nHost:%s:%d\r\n\r\n",IDPpath, pstr->c_str(), "passport.sohu-rd.com", port);

    // 发送https头，内容与http相同
    SSL_write(ssl, header, strlen(header));   

    int  n;
    char buf[BUF_LEN]; // 临时缓冲区
    while((n = SSL_read(ssl, buf, BUF_LEN - 1)) > 0)
    {
        // 每次将临时缓冲区中的数据
        // 拷贝到response_buf中
        buf[n] = '\0';
        write(1, buf, n);
        strcat(response_buf, buf);
    }

    if(n != 0)
    {
        err_quit("SSL read failed:%s\n",ERR_reason_error_string(ERR_get_error()));
    }   

    // 关闭 ssl
    ret = SSL_shutdown(ssl);
 
    if(ret != 1)
    {
        close(fd);
        err_quit("SSL shutdown failed:%s\n",ERR_reason_error_string(ERR_get_error()));
    }

    // 关闭套接字描述符  
    close(fd);

    // 清理SSL资源
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    ERR_free_strings();

    //// 处理SAML Response 
    apr_pool_t* pool;
    apr_pool_create(&pool, NULL);

    // 得到GET回来的数据Body
    char* p_response_body = NULL;
    p_response_body = strstr(response_buf, "\r\n\r\n");
    p_response_body += strlen("\r\n\r\n");

    drop(p_response_body); // 去掉多余的换行符

    char* res_decoded = NULL;
    res_decoded = (char *) apr_palloc(pool, 1 + apr_base64_decode_len(p_response_body));
    int l = apr_base64_decode(res_decoded, p_response_body);
    res_decoded[l] = '\0';

    char* response = NULL;
    response = new char[strlen(res_decoded) + 1];
    strcpy(response, res_decoded);

    delete[] request_buf;
    apr_pool_destroy(pool);

    return response;
}

static char*
getSAMLAuthDecisionStatement(char* response)
{
    xmlDocPtr  response_doc  = NULL;
    xmlNodePtr response_root = NULL;
    xmlNodePtr assertion     = NULL;

    response_doc = xmlParseDoc((xmlChar*)response);
    response_root = xmlDocGetRootElement(response_doc);

    if(!response_doc)
       return NULL;

    xmlNodePtr child = response_root->children;
    while(child)
    {
       if(child->name && xmlStrcmp(child->name, (xmlChar*)"Assertion") == 0)
       {
           break;
       }
       child = child->next;
    }

    assertion = child;

    xmlNodePtr Auth_decision_statement = NULL;

    child = assertion->children;
    while(child)
    {
       if(child->name && xmlStrcmp(child->name, (xmlChar*)"AuthorizationDecisionStatement") == 0)
       {
           break;
       }
       child = child->next;
    }

    Auth_decision_statement = child;

    char* statement = (char*)xmlGetProp(Auth_decision_statement, (const xmlChar*)"Decision");

    xmlFreeDoc(response_doc);

    return statement;
}

/**************************************************************************
 * 对外接口
 * 用于解析SAMLResponse的内容，得到写cookie所需要的参数
 */
extern "C" int 
get_decision(char* username, char* resource, char* action, char* idp_addr, char* idp_path, unsigned int port)
{
    if(username == NULL || resource == NULL || action == NULL)
       return 1;
    
    XMLPlatformUtils::Initialize();
    apr_initialize();

    SAMLRequest* request = 
    make_auth_decision_request(username, resource, action);

    if(request == NULL)
    {
       cerr << "Make Saml Request Failed!" << endl;
       return 0;
    }

    char* response =
    get_auth_decision_response(request, idp_addr, idp_path, port);

    char* decision = 
    getSAMLAuthDecisionStatement(response);

    int rv = 0;
    if(strcmp(decision, "Permit") == 0)
       rv = 1;
    else
       rv = 0;

    delete   request;
    delete[] response;
    xmlFree(decision);

    apr_terminate();
    XMLPlatformUtils::Terminate();

    return rv;
}

/**************************************************************************/
