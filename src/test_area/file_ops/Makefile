#
#
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../fileops.o ../test_utils.o ../../string.o ../../mem_tracker.o ../../array.o ../../hash.o

all: test_01

test_01:	test_01.c $(LIBS)
	$(CC) $(CFLAGS) -o test_01 test_01.c $(LIBS)

clean:
	@rm *_test
