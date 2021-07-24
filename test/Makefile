UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
	CC = clang
else ifeq ($(UNAME),Linux)
	CC = gcc
endif

CFLAGS += -g -Wall -Wextra -Werror
CFLAGS += -std=c11 -D_POSIX_C_SOURCE=1

run: test
	./test

test: main.c ../sex.h ../sex.c ../sexprint.c
	$(CC) $^ -o $@ $(CFLAGS)

