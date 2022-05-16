#
#
#

CC=gcc
AFC_CFLAGS=`afc-config --cflags`
AFC_LIBS=`afc-config --libs`

CFLAGS=-Wall $(AFC_CFLAGS)
LIBS=$(AFC_LIBS) -lpthread -ldl

libs: mysql.so postgresql.so

mysql.so: mysql.c 
	$(CC) $(CFLAGS) -fPIC -nostartfiles -shared -Wl,-soname,mysql.so.1 -o mysql.so  mysql.c -lc 

postgresql.so: postgresql.c 
	$(CC) $(CFLAGS) -fPIC -nostartfiles -shared -Wl,-soname,postgresql.so.1 -o postgresql.so  postgresql.c -lc 

mysql.o:	mysql.c
	$(CC) $(CFLAGS) -o mysql.o -c mysql.c

test:		mysql.test postgresql.test

mysql.test:	mysql.c
	$(CC) $(CFLAGS) -g -DTEST_CLASS -o mysql.test mysql.c $(LIBS) -lmysqlclient

postgresql.test:	postgresql.c
	$(CC) $(CFLAGS) -g -DTEST_CLASS -o postgresql.test postgresql.c $(LIBS) -lpq

clean:
	@rm -f *.so


