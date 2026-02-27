#
# AFC Test Suite Makefile
#

CC=gcc
CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -iquote../src

AFC_LIB=../src/libafc.a
PCRE_LIB=../src/pcre/libpcre.a
LIBS=$(AFC_LIB) $(PCRE_LIB) -lssl -lcrypto -lpthread -ldl -lm

TESTS = test_base test_string test_mem_tracker \
        test_array test_list test_hash test_dictionary test_string_list \
        test_bin_tree test_btree test_avl_tree test_circular_list test_tree \
        test_base64 test_md5 test_date_handler test_readargs test_regexp \
        test_fileops test_cgi_manager test_dirmaster \
        test_dynamic_class test_cmd_parser test_threader \
        test_inet_client test_inet_server \
        test_smtp test_http_client test_pop3
# NOTE: test_ftp_client excluded - ftp_client.c has build errors
#       (references undefined afc_inet_client_get_binary)

all: test_utils.o $(TESTS)

test_utils.o: test_utils.c test_utils.h
	$(CC) -c $(CFLAGS) test_utils.c -o test_utils.o

test_%: test_%.c test_utils.o $(AFC_LIB)
	$(CC) $(CFLAGS) -o $@ $< test_utils.o $(LIBS)

clean:
	@rm -f *.o $(TESTS)

run: all
	@./run_all.sh

.PHONY: all clean run
