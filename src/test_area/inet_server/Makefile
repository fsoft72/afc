#
#
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../string.o ../../array_master.o ../../hash_master.o ../../inet_server.o ../../mem_tracker.o

all: test_01

test_01:	test_01.c $(LIBS)
	$(CC) $(CFLAGS) -o test_01 test_01.c $(LIBS)

clean:
	@rm *_test
