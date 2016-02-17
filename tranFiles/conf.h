#include<stdint.h>
#include<pthread.h>

struct file_link
{
    char name[255];
    time_t modify_time;
    struct file_link *next;
};
struct file_link_header
{
    struct file_link *next;
    uint32_t num;
};
struct global_conf
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint32_t pthread_num;
    char monitor_dir[255];
    char rsync_dest[1024];
    char cmd[1024];
    char log_file[255];
    char check_exists[1024];
    struct file_link_header *header;
};
void parse_config(const char* conf_file, struct global_conf * ret_conf);
