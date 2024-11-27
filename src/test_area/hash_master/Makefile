#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../array.o ../../hash_master.o ../../string.o ../test_utils.o ../../mem_tracker.o
LIBS=-ldl

PROGS=test_01 test_02 test_03

all: $(PROGS)

test_%: test_%.c $(OBJS)
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

clean:
	@rm $(PROGS)
