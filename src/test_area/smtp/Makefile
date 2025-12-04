#
# AFC SMTP Test Makefile
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../string.o ../../mem_tracker.o ../../inet_client.o ../../smtp.o ../../base64.o ../../dictionary.o ../../hash.o ../../array.o ../../list.o ../test_utils.o
LDFLAGS=-lssl -lcrypto

all: test_01

test_01:	test_01.c $(LIBS)
	$(CC) $(CFLAGS) -o test_01 test_01.c $(LIBS) $(LDFLAGS)

clean:
	@rm -f test_01

.PHONY: all clean
