AM_CPPFLAGS=-g -I$(top_builddir)
AM_CFLAGS=-g -I$(top_builddir)


noinst_PROGRAMS=download
download_SOURCES=download.cpp
download_LADD =
download_LDFLAGS=-L./utils -L./down -L./threadManage -lcurl -lpthread -ldown -lthreadManage -lutils
SUBDIRS=down threadManage utils


noinst_PROGRAMS += parser_client
parser_client_SOURCES = parser_client.cpp
parser_client_LADD =
parser_client_LDFLAGS =-L./parser_request  -L./threadManage -L./utils -L./mysql -L./parser_thrift -lparser_request -lparser_thrift -lthreadManage -lmysql -lutils -lpthread -lthrift -lmysqlclient -lm -ldl 
SUBDIRS+=parser_thrift parser_request mysql 
