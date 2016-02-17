#include <stdio.h>
#include <assert.h>

#include "apr_general.h"
#include "apr_pools.h"
#include "apr_strings.h"

int test_str(){
    apr_pool_t *mp;
    apr_status_t rv;
    
    rv = apr_initialize();
    if(APR_SUCCESS != rv){
        assert(0);
        return -1;
    }

    apr_pool_create(&mp, NULL);

    const char* str1 = "str1------1";
    const char* str2 = "str2------2";
    const char* str3 = "str3------3";

    char *cat_str = apr_pstrcat(mp, str1, str2, str3, NULL);
    fprintf(stderr,"%s\n",cat_str);
    
    int natcmp = apr_strnatcmp(str1,str2);
    fprintf(stderr,"apr_strnatcmp=%d\n",natcmp);

    char* buf1 = apr_pstrdup(mp,cat_str);
    fprintf(stderr,"apr_pstrdup=%s\n",buf1);

    char* test_token = apr_palloc(mp,100);
    const char* tmp = "wendao\nliuyongshuai\nabc\naaaaaaaaa";
    test_token = apr_pstrdup(mp,tmp);
    char* token;
    char* last;
    fprintf(stderr,"%s\n",test_token);
    token = apr_strtok(test_token,"\n",&last);
    fprintf(stderr,"%s\n",token);

    int abc = 1569784;
    char* abc_buf = apr_palloc(mp,10);
    fprintf(stderr,"%s\n",apr_itoa(mp,abc));

    long ddd = 1569784;
    char* ddd_buf = apr_palloc(mp,10);
    fprintf(stderr,"%s\n",apr_ltoa(mp,ddd));

    apr_pool_destroy(mp);
    apr_terminate();
    return 0;
}
