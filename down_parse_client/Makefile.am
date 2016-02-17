AM_CPPFLAGS=-g -ggdb -Wall -I$(top_builddir) -I./down_client -I./utils -I./task_server_thrift -I./parse_client_thrift -I./parse_client -I./mysql -I./down_client_thrift -I./jsoncpp -I/usr/local/mysql/include -I/usr/local/include 


noinst_PROGRAMS = down_parse_client
down_parse_client_SOURCES = down_parse_client.cpp clientConfig.h clientConfig.cpp
down_parse_client_LDADD = -ltask_server_thrift -lparse_client -ldown_client -ldown_client_thrift -lparse_client_thrift -lmysql -lutils -lcurl -lmysqlclient -levent  -lpthread -lthrift -lthriftnb  -luuid -lm -ljsoncpp
down_parse_client_LDFLAGS = -L./task_server_thrift -L./parse_client -L./down_client -L./down_client_thrift -L./parse_client_thrift -L./jsoncpp -L./mysql -L./utils -L/usr/local/mysql/lib -L/usr/local/lib 
SUBDIRS = utils task_server_thrift parse_client_thrift parse_client down_client_thrift down_client mysql

