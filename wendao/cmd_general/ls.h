struct ls_finfo{
    long mtime;
    char modeBuf[11];
    int nlink;
    char uname[20];
    char gname[20];
    long size;
    char fname[255];
    struct ls_finfo *next;
};
int cmd_ls(int argc, char *argv[]);
int sortList();
