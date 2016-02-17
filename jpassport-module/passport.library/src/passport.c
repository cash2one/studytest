#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>

/**
 *  ����ǩ��ʱ����Ҫ��˽��
 */
static EVP_PKEY* load_private_key(const char* filename, const char* password)
{
    BIO* bp;
    EVP_PKEY* pkey;

    // ��ʼ�� BIO
    if ((bp = BIO_new(BIO_s_file())) == NULL){
        return NULL;
	}

    // �趨Ҫ��ȡ��˽��λ��
    BIO_read_filename(bp, filename);

    // ����˽��
    if ((pkey = PEM_read_bio_PrivateKey(bp, NULL, NULL, (char *)password)) == NULL)
    {
         BIO_free(bp);
         return NULL;
    }

    BIO_free(bp);

    return pkey;
}

/**
 * ǩ������
 * ע�⣡���ص�ǩ���ַ�����Ҫ��free���ͷ�
 * msg: ����ǩ��������
 * private_key_addr: ˽�״�ŵ�λ��
 */
char* sign(char* msg, char* private_key_addr, char* password)
{
    int   i;
    char* sign_value_str = NULL; // ǩ��������

    EVP_MD_CTX sign_ctx;
    EVP_PKEY*  privKey = NULL;

    /* ��ź���̎������Ҫ�� private key һ���L */
    unsigned char sign_value[1024];
    unsigned int  sign_len;

    OpenSSL_add_all_algorithms();
    if ((privKey = load_private_key(private_key_addr, password)) == NULL)
    {
        printf("unable to load private key\n");
        EVP_MD_CTX_cleanup(&sign_ctx);

        return NULL;
    }

    EVP_SignInit(&sign_ctx, EVP_sha1());
    EVP_SignUpdate(&sign_ctx, msg, strlen(msg));
    EVP_SignFinal(&sign_ctx, sign_value, &sign_len, privKey);

    sign_value_str = malloc(sign_len * 2 + 1);

    sign_value_str[0] = 0;

    for(i=0; i < sign_len; i++)
    {
        sprintf(sign_value_str + i * 2, "%02X", sign_value[i]);
    }

    EVP_PKEY_free(privKey);
    EVP_MD_CTX_cleanup(&sign_ctx);

    return sign_value_str;
}

/** �õ�ĳ�������ӽ��
 *  node: ���ָ��
 *  tagName: �ӽ���tag����
 */
static xmlNodePtr get_element_by_tagname(xmlNodePtr node, const xmlChar* tagname)
{
    if(node == NULL){
       return NULL;
	}

     xmlNodePtr child = node->children; 
     while(child)
     {
        if(child->name && xmlStrcmp(child->name, tagname) == 0)
        {
            return child;
        }
        child = child->next;
      }
      return NULL;
}

/**
 *  ����SAML�ĸ����
 */
static xmlDocPtr parse_saml_doc(const xmlChar* saml_buffer)
{
   xmlDocPtr doc = NULL;
   doc = xmlParseDoc(saml_buffer);
   if(!doc){
       return NULL;
	}

   return doc;
}

/**
 * assertion ��<Response>�ĵ�һ���ӽ��
 */
static xmlNodePtr get_assertion(xmlNodePtr root)
{
   xmlNodePtr assertion = NULL;
   assertion = get_element_by_tagname(root, "Assertion");
   return assertion;
}

static char* get_name_identifier(xmlNodePtr assertion)
{
    xmlNodePtr authentication_statement = NULL;
    xmlNodePtr subject = NULL;
    xmlNodePtr name_identifier = NULL;

    authentication_statement = get_element_by_tagname(assertion, "AuthenticationStatement");
    if(authentication_statement != NULL)
    {
       subject = get_element_by_tagname(authentication_statement, "Subject");
       if(subject != NULL)
       {
          name_identifier = get_element_by_tagname(subject, "NameIdentifier");
          if(name_identifier != NULL){
            return  xmlNodeGetContent(name_identifier);
			}
       }
    }
    
    return NULL;
}

/**
 *  ע�⣬notbefore��notonorafter��Ҫ�ֶ���xmlFree�ͷ�
 *  ����0��ʾ����������-1��ʾʧ��
 */
static int get_time(xmlNodePtr assertion, char** notbefore, char** notonorafter)
{
    xmlNodePtr conditions = NULL;

    if(assertion == NULL) {
		return -1;
	}

    conditions = get_element_by_tagname(assertion, "Conditions");
    if(conditions != NULL)
    {
        (*notbefore) = (char*)xmlGetProp(conditions, (const xmlChar*)"NotBefore");
        (*notonorafter) = (char*)xmlGetProp(conditions, (const xmlChar*)"NotOnOrAfter");

        if(*notbefore && *notonorafter){
             return 0;
		}
    }

    return -1;
}

/**************************************************************************
 * ����ӿ�
 * ���ڽ���SAMLResponse�����ݣ��õ�дcookie����Ҫ�Ĳ���
 */
int get_saml_cookie_parameter(char* response, 
							  char** username, 
                              char** notbefore, 
                              char** notafter, 
                              char** signvalue, 
                              const char* privkeyaddr,
                              const char* password)
{
    int rv;
    xmlDocPtr response_doc = NULL;
    xmlNodePtr response_root = NULL;
    xmlNodePtr assertion = NULL;

    char* strsum = NULL;
    int total_length = 0;

    response_doc = parse_saml_doc(response);
    if(response_doc == NULL) {
		return -1;
	}

    response_root = xmlDocGetRootElement(response_doc);
    if(response_root == NULL) 
    {
       xmlFreeDoc(response_doc);
       return -2;
    }

    assertion = get_assertion(response_root);
    if(assertion == NULL) 
    {
       xmlFreeDoc(response_doc);
       return -3;
    }

    // �õ��û���
    *username = get_name_identifier(assertion);
    // �õ�ʱ���������
    get_time(assertion, notbefore, notafter);

    if(*username == NULL || *notbefore == NULL || notafter == NULL)
    {
      xmlFreeDoc(response_doc);
      return -4;
    }

    xmlFreeDoc(response_doc);

    total_length = strlen(*username) + strlen(*notbefore) + strlen(*notafter) + 1;
    strsum = (char*)malloc(total_length * sizeof(char));
    strsum[0] = '\0';

    // �������ַ������
    strcat(strsum, *username);
    strcat(strsum, *notbefore);
    strcat(strsum, *notafter);

    *signvalue = sign(strsum, privkeyaddr, password);
    free(strsum);

    if(*signvalue == NULL){
        return -5;
	}

    return 0;
}

int free_saml_cookie_paramter(char* username, char* notbefore, char* notafter, char* signvalue)
{
    xmlFree((xmlChar*)username);
    xmlFree((xmlChar*)notbefore);
    xmlFree((xmlChar*)notafter);

    // signvalue����malloc����ģ��ʱ�����free�ͷ�
    free(signvalue);

    return 0;
}
/**************************************************************************/

