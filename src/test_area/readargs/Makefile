#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.c ../../nodemaster.c ../../stringnode.c ../../string.c ../../readargs.c
LIBS=../../base.o ../../nodemaster.o ../../stringnode.o ../../string.o ../../readargs.o ../test_utils.o  ../../mem_tracker.o

PROGS=test_01 test_02 test_03

all: $(PROGS)

leak:	test_01_leak

test_%: test_%.c $(LIBS)
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(LIBS)

test_01_leak: test_01.c
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -fcheck-memory-usage -o $@ $(OBJS) $< -lmpatrol -lbfd -liberty ../test_utils.o

clean:
	@rm $(PROGS)
