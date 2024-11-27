#
#
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../string.o ../../array.o ../../fs_tree.o ../../stringlist.o ../../list.o ../test_utils.o

all: test_01 test_02 test_03

test_01:	test_01.c $(LIBS)
	$(CC) $(CFLAGS) -o test_01 test_01.c $(LIBS)

test_02:	test_02.c $(LIBS)
	$(CC) $(CFLAGS) -o test_02 test_02.c $(LIBS)

test_03:	test_03.c $(LIBS)
	$(CC) $(CFLAGS) -o test_03 test_03.c $(LIBS)

clean:
	@rm *_test
