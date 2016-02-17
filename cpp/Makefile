SRCS:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp,%.o,$(SRCS))
CPPFLAGS=-std=gnu++11 -g -Wall -I/usr/include -I/usr/include/libxml2 -L/usr/local/mysql5.6/lib -lmysqlclient -lpthread -lm -ldl -lboost_regex -lxml2
CC=g++
all:${OBJS}
	g++ -g ${OBJS} -o main ${CPPFLAGS} 

.PHONY:clean
clean:
	-rm *.o main
