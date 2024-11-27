#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../array.o ../../hash_master.o ../../string.o  ../../dictionary.o ../../dynamic_class.o ../../dynamic_class_master.o ../test_utils.o

LIBS=-ldl

all: test_01

test_%: test_%.c
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

clean:
	@rm *_test
