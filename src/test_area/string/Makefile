#
#
#

CC=gcc
CFLAGS=-g -Wall
LIBS=../../base.o ../../string.o ../test_utils.o ../../mem_tracker.o

all: test_01 test_02 test_03

leak: test_01_leak test_02_leak

test_01:	test_01.c $(LIBS)
	$(CC) $(CFLAGS) -o test_01 test_01.c $(LIBS)

test_02:	test_02.c $(LIBS)
	$(CC) $(CFLAGS) -o test_02 test_02.c $(LIBS)

test_03:	test_03.c $(LIBS)
	$(CC) $(CFLAGS) -o test_03 test_03.c $(LIBS)

test_01_leak: test_01.c
	@echo "Building Leak: $@"
	@$(CC) $(CFLAGS) -fcheck-memory-usage -o $@ $< -lmpatrol -lbfd -liberty $(LIBS)

test_02_leak: test_02.c
	@echo "Building Leak: $@"
	@$(CC) $(CFLAGS) -fcheck-memory-usage -o $@ $< -lmpatrol -lbfd -liberty $(LIBS)

clean:
	@rm *_test
