#
#
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../list.o ../../string.o ../test_utils.o  ../../mem_tracker.o

all: test_02 test_01

test_%: test_%.c
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	@rm *_test
