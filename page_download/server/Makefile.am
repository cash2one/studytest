AM_CPPFLAGS=-g -I$(top_builddir)
AM_CFLAGS=-g -I$(top_builddir)

noinst_PROGRAMS = task
task_SOURCES=task.cpp
task_LADD =
task_LDFLAGS=-L./utils -L./grab -L./mysql -L./threadManage -lcurl -lpthread -lgrab -lmysql -lthreadManage -lutils -lmysqlclient -lm -ldl -lxml2
SUBDIRS=mysql grab threadManage utils

noinst_PROGRAMS += monitor
monitor_SOURCES= server.cpp
monitor_LDADD = -lthrift
monitor_LDFLAGS=-L./utils -L./server -lserver  -lutils
SUBDIRS += server 
