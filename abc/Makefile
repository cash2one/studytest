C_SRCS:=$(wildcard *.c)
CPP_SRCS:=$(wildcard *.cpp)
ALL_FILE=$(wildcard *)


define wendao
for cf in $(C_SRCS);do \
	bin=$${cf%\.c}; \
	rm -f $$bin; \
	echo "gcc -g -Wall $$cf -o $$bin"; \
	gcc $$cf -g -Wall -o $$bin -lpthread -lcurl; \
done
for cf in $(CPP_SRCS);do \
	bin=$${cf%\.cpp}; \
	rm -f $$bin; \
	echo "g++ -g -Wall $$cf  -o $$bin"; \
	g++ $$cf -g -Wall -std=c++0x -o $$bin -lpthread; \
done
endef

define Mclean
for af in $(ALL_FILE);do \
	if [ -f $${af}.c -o -f $${af}.cpp ];then \
		echo "rm -f $$af"; \
		rm -f $$af; \
	fi; \
done
endef


all:${SRCS}
	@$(call wendao)

.PHONY:clean
clean:
	@$(call Mclean)
