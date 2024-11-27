#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../string.o ../../array.o ../../dictionary.o ../../hash.o ../test_utils.o  ../../mem_tracker.o
PROGS=test_01 test_02 test_03

all: $(PROGS)

test_%: test_%.c $(OBJS)
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(OBJS)

clean:
	@rm $(PROGS)
