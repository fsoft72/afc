#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../string.o ../../mem_tracker.o ../test_utils.o ../../circular_list.o
PROGS=test_01 test_02 test_03 

all: $(PROGS) 

test_%: test_%.c $(OBJS)
	@echo "Building: $@"
	$(CC) $(CFLAGS) -o $@ $< $(OBJS)

clean:
	@rm $(PROGS) 2>/dev/null
