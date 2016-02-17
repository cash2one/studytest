AM_CPPFLAGS=-g -I$(top_builddir)
AM_CFLAGS=-g -I$(top_builddir)
noinst_PROGRAMS = client

client_SOURCES= grab_page.cpp \
				grab_page.h \
				page_download_constants.cpp \
				page_download_constants.h \
				page_download_types.cpp \
				page_download_types.h \
				client.cpp
client_LDADD = -lthrift -lpthread -lm -ldl
client_LDFLAGS = -I/usr/local/include
