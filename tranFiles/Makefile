CFLAGS=-g -Wall
CC=gcc
SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
all:${OBJS}
	gcc -g -Wall ${OBJS} -o trans -lpthread

.PHONY:clean
clean:
	-rm *.o trans
