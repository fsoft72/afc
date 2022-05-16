#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../string.o ../../regexp.o  ../test_utils.o  ../../mem_tracker.o ../../pcre/libpcre.a
LIBS=-ldl

FILES=test_01 test_02
all: $(FILES)

test_%: test_%.c  $(OBJS)
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

clean:
	@rm $(FILES)
