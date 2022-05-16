#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../string.o ../../mem_tracker.o  ../../array_master.o ../../dictionary.o ../../hash_master.o
PROGS=test_01 test_02 test_03 test_04 test_05 test_06 test_07 test_08

all: $(PROGS) 

test_%: test_%.c $(OBJS)
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(OBJS)

clean:
	@rm $(PROGS) 2>/dev/null
