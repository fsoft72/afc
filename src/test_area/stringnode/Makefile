#
#
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../string.o ../../list.o ../../stringnode.o ../test_utils.o ../../mem_tracker.o

all: test_01

test_01:	test_01.c $(LIBS)
	$(CC) $(CFLAGS) -o test_01 test_01.c $(LIBS)

clean:
	@rm *_test
