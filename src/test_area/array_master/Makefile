#
#
#

CC=gcc
CFLAGS=-g -Wall -DDEBUG_MEM
OBJS=../../base.o ../../string.o ../../array.o ../../mem_tracker.o ../test_utils.o
PROGS=test_01 test_02 test_03 test_04 test_05

all: $(PROGS)

test_%: test_%.c $(OBJS)
	@echo "Building: $@"
	$(CC) $(CFLAGS) -o $@ $< $(OBJS)

clean:
	@rm -f $(PROGS) 2>/dev/null
