#include "mod_newpassport.h"
#include "httpdh.h"

int get_shire_addr( request_rec *r, char** shire_addr );
int is_passport_enabled(request_rec *r);
void get_server_conf(request_rec *r, passport_server_conf** conf);
void get_dir_conf(request_rec *r, passport_dir_conf** conf);
short int get_idp_https_port(request_rec *r);
int get_idp_auth_hostname( request_rec *r, char** hostname );
int get_idp_auth_path( request_rec* r, char** path );
int is_passport_short_username(request_rec *r);


const char* mod_newpassport_set_filter(cmd_parms *cmd, void *mconfig, const char* arg);
const char* mod_newpassport_set_short_username(cmd_parms *cmd, void *mconfig, int arg);
const char *mod_newpassport_set_passport_enable(cmd_parms *cmd, void *mconfig, int arg);
const char* mod_newpassport_set_idp_url_address(cmd_parms *cmd, void *mconfig, const char *addr);
const char* mod_newpassport_set_certificate_filename(cmd_parms *cmd, void *mconfig, const char *addr);
const char* mod_newpassport_set_authentication_url(cmd_parms *cmd, void *mconfig, const char *addr);
const char* mod_newpassport_set_private_key_address(cmd_parms *cmd, void *mconfig, const char *addr);
const char* mod_newpassport_set_private_key_password(cmd_parms *cmd, void *mconfig, const char *addr);
void *mod_newpassport_merge_per_server(apr_pool_t *p, void *basev, void *addv);
void *mod_newpassport_create_per_server(apr_pool_t *p, server_rec *d);
void *mod_newpassport_merge_per_dir(apr_pool_t *p, void *basev, void *addv);
void *mod_newpassport_create_per_dir(apr_pool_t *p, char *d);