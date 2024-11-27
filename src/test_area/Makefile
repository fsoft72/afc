#
#
#

CC=gcc
CFLAGS=-g -Wall

DIRS=dir_array dir_string dir_readargs dir_list dir_hash_master

all: test_utils.o $(DIRS)

dir_array:
	@cd array; make

dir_string:
	@cd string; make

dir_readargs:
	@cd readargs; make

dir_list:
	@cd list; make

dir_hash_master:
	@cd hash_master; make

%.o : %.c
	@echo "Building: $@"
	$(CC) $(CFLAGS) -o $@ -c $<

test: test_utils

test_utils: test_utils.c
	$(CC) $(CFLAGS) -DTEST_CLASS -o test_utils test_utils.c

clean:
	@rm *_test
