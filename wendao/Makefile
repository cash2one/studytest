export MAKEROOT:=$(shell pwd)
TARGET_DIRS:=cmd_general \
             cmd_manager \
			 cmd_net

OBJECTS=cmd_general/*.o cmd_manager/*.o cmd_net/*.o *.o

include ./common.mk

define build_obj
for SubDir in $(TARGET_DIRS);do \
    if ! [ -d $$SubDir ]; then \
        echo "The $$SubDir is not exist !"; \
        exit 11; \
    fi; \
    echo "Building $$SubDir ..."; \
    make -C $$SubDir ; \
    if [ $$? -ne 0 ]; then \
echo "Building $$SubDir is failed !"; \
        exit 12; \
    fi; \
done
endef

SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
all:${OBJS}
	@$(call build_obj)
	${CC} -o wendao ${OBJECTS}
clean:
	-rm ./wendao ${OBJECTS}
