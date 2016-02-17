CC:=gcc
INCLUDE_DIR:= -I./ \
	-I$(MAKEROOT)/cmd_general \
	-I$(MAKEROOT)/cmd_manager \
	-I$(MAKEROOT)/cmd_net

CFLAGS:=$(INCLUDE_DIR) -g -Wall
CC=gcc
%.o:%.c
	${CC} ${CFLAGS} -c $< -o $@
