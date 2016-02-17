noinst_PROGRAMS = main
main_SOURCES = main.c
main_LDADD = -L/usr/local/mysql5.5/lib -L./utils -L./send_so -lmysqlclient -lutils -lsend
main_LDFLAGS = -I/usr/local/mysql5.5/include -I./utils -I./send_so
SUBDIRS=utils send_so
