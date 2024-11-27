#
#
#

CC=gcc
CFLAGS=-g -Wall
OBJS=../../base.o ../../string.o ../../array.o ../../list.o ../../hash_master.o ../../dictionary.o  ../../dynamic_class.o \
     ../../dynamic_class_master.o  ../../cmd_parser.o ../../stringnode.o ../../readargs.o

LIBS= -ldl -lpthread

PROGS=test_01

all: $(PROGS)

test_%: test_%.c $(OBJS)
	@echo "Building: $@"
	@$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(OBJS)

clean:
	@rm $(PROGS) 2>/dev/null
