AM_CPPFLAGS=-g -ggdb -Wall -I$(top_builddir) -I./utils -I./down_action -I./thrift_gen 


noinst_PROGRAMS=downServer
downServer_SOURCES=downServer.cpp
downServer_LDADD =-ldown_action  -lutils -lthrift_gen -lcurl -lthrift -luuid
downServer_LDFLAGS=-L./down_action  -L./utils  -L./thrift_gen -L/usr/local/lib 
SUBDIRS=down_action  utils thrift_gen


