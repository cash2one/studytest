lib_LTLIBRARIES=libmod_newpassport.la
libmod_newpassport_la_SOURCES = gen_sign.c \
				gen_sign.h \
				compress.c \
				compress.h \
				config_parser.c \
				config_parser.h \
				utils.c \
				utils.h \
				mod_newpassport.c \
				mod_newpassport.h \
				httpdh.h \
				response_tool.c \
				saml_tool.c 
libmod_newpassport_la_LIBADD=-lxml2 -lssl -lz -lapr-1 -laprutil-1 -L/usr/local/apache2/lib
AM_CFLAGS=-g -I$(top_srcdir)/newpassport -I/usr/include/apr-1/ -I/usr/local/apache2/include/ -I/usr/include/libxml2  
