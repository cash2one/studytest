void testUtils();
void qsortString(char *strArr[],const int leftIndex,const int rightIndex);
int parseStatMode(char buf[],int bufsize,mode_t st_mode);
int uid2name(char unameBuf[],int unameBufSize,uid_t uid,char gnameBuf[],int gnameBufSize);
int gid2gname(char buf[], int bufsize, gid_t gid);
void trim(char buf[],char ch);
int check_string(const char* str,char type);
int uname2uid(uid_t uid[],gid_t gid[],const char uname[]);
