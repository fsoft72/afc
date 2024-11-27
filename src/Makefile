#
# AFC Makefile
#
# (C)Copyright 2001 Fabio Rotondo (fabio@rotondo.it)
#
# This software is distributed under the GPL (v2 or +) licence
# For more info, visit http://www.gnu.org
#

CC=gcc
AR=ar

#
# Modify these params if the commands are somewhere else
#

SED=sed
CP=cp
MV=mv
LN=ln -sf
RM=rm -f
CHMOD=chmod

CFLAGS=-fPIC -g -Wall -Wextra -O2 -Wstrict-prototypes -Wno-unused-label -Wno-unused-parameter -Wno-unused-but-set-variable

ifeq ($(DEFINE),MINGW)
# MINGW version has only a subset of AFC full capabilities
CFLAGS+=-DMINGW

OBJS=string.o base.o base64.o list.o array.o cgi_manager.o dictionary.o hash_master.o \
     mem_tracker.o readargs.o regexp.o stringlist.o dynamic_class.o \
     threader.o date_handler.o md5.o fileops.o avl_tree.o tree.o

else
# This is the full pack

CFLAGS+=-O2

OBJS=string.o base.o base64.o list.o array.o cgi_manager.o dictionary.o dirmaster.o hash_master.o \
     mem_tracker.o readargs.o regexp.o stringlist.o dynamic_class.o dynamic_class_master.o \
     cmd_parser.o threader.o inet_client.o inet_server.o date_handler.o md5.o bin_tree.o dbi_manager.o \
     circular_list.o btree.o avl_tree.o  fileops.o tree.o\
	pop3.o
endif

LIBFLAGS=-shared

AFCVER=2
AFCREV=3
AFCMK=0

#
# Modify these params to copy files in different locations
#
LIBDIR=/usr/local/lib
BINDIR=/usr/local/bin
INCDIR=/usr/local/include

AFC_LIBS=`afc-config --libs`

TESTS=bin_tree.test

all: libpcre $(OBJS) static lib

mingw_all: mingw_libpcre $(OBJS) mingw_static

mingw_libpcre:
	@make clean; cd pcre; make clean; make mingw_all

mingw_static: $(OBJS)

	@echo "Creating Static library..."
	@$(AR) rcs libafc.a $(OBJS) pcre/libpcre.a #/mingw/lib/pthreadGC.a

test:	$(TESTS)

libpcre:
	@cd pcre; make


static: $(OBJS)
	@echo "Creating Static library..."
	@$(AR) rcs libafc.a $(OBJS) pcre/libpcre.a

lib: $(OBJS)
	@echo "Creating Shared library..."
	@$(CC) $(LIBFLAGS) -Wl,-soname,libafc.so.$(AFCVER) -o libafc.so.$(AFCVER).$(AFCREV).$(AFCMK) $(OBJS) pcre/libpcre.a

checkdirs:
	@echo "Checking "
	@echo "   ... library dir"
	@if ! [ -d $(LIBDIR) ]; then echo "Install directory not found: $(LIBDIR)"; exit 1; fi
	@echo "   ... binary dir"
	@if ! [ -d $(BINDIR) ]; then echo "Install directory not found: $(BINDIR)"; exit 1; fi
	@echo "   ... include dir"
	@if ! [ -d $(INCDIR) ]; then echo "Install directory not found: $(INCDIR)"; exit 1; fi

bins:
	@echo "Copying binaries"
	@$(SED) -e 's|@LIBDIR@|$(LIBDIR)|g' -e 's|@INCDIR@|$(INCDIR)|g' bin/afc-config > $(BINDIR)/afc-config
	@$(CHMOD) 755 $(BINDIR)/afc-config
	@$(CP) bin/afc_start $(BINDIR)/.

includes:	#lib_easygui_includes
	@echo "Copying header files"
	@if [ -d $(INCDIR)/afc ]; then echo "AFC Directory already exists: just copying files..."; else mkdir $(INCDIR)/afc; fi
	@cp *.h $(INCDIR)/afc
	@install -d $(INCDIR)/afc/pcre
	@install pcre/*.h $(INCDIR)/afc/pcre/


install: checkdirs static lib bins includes
	@echo "Installing"
	@echo "   ... shared library"
	@$(MV) libafc.so* $(LIBDIR)
	@echo "   ... static library"
	@$(MV) libafc.a $(LIBDIR)
	@echo "   ... creating symlinks"
	@$(RM) $(LIBDIR)/libafc.so
	@$(LN) $(LIBDIR)/libafc.so.$(AFCVER) $(LIBDIR)/libafc.so
	ldconfig
	@echo "   ... DONE!"

#lib_easygui:
#	@cd easygui; make lib

#lib_easygui_install:	lib_easygui
#	@cd easygui; make install

#lib_easygui_includes:
#	@cd easygui; make includes
#

cpp:
	@cd afcpp; make

cpp_install: cpp cpp_lib
	@echo "Installing"
	@echo "   ... AFC++ library"
	@$(MV) afcpp/libafcpp.so* $(LIBDIR)
	@$(RM) $(LIBDIR)/libafcpp.so
	@$(LN) $(LIBDIR)/libafcpp.so.$(AFCVER) $(LIBDIR)/libafcpp.so
	@echo "   ... DONE!"

cpp_lib:
	@cd afcpp; make lib

clean:
	@$(RM) *.o 2>/dev/null
	@$(RM) libafc.a 2>/dev/null
	@$(RM) libafc.so* 2>/dev/null
	@$(RM) afcpp/*.o 2>/dev/null
	@$(RM) afcpp/libafcpp.so* 2>/dev/null
	@find -name "*.test" -type f -exec $(RM) {} \;
	@cd pcre; make clean
	@cd dbi; make clean

%.o : %.c %.h
	@echo "Building $@"
	$(CC) -c $(CFLAGS) $< -o $@

base.test: base.c
	@$(CC) -DTEST_CLASS -o base.test $(CFLAGS) base.c string.o

array.test:	array.c array.h
	@$(CC) -DTEST_CLASS -o array.test $(CFLAGS) array.c string.o base.o mem_tracker.o

dirmaster.test:	dirmaster.c
	@$(CC) -DTEST_CLASS -o dirmaster.test $(CFLAGS) dirmaster.c string.o array.o

cmdparser.test: cmd_parser.c cmd_parser.h
	@$(CC) -DTEST_CLASS -o cmdparser.test $(CFLAGS) cmd_parser.c string.o base.o list.o array.o cgi_manager.o dictionary.o dirmaster.o hash_master.o readargs.o regexp.o stringlist.o fileops.o dynamic_class.o dynamic_class_master.o -ldl

threader.test:	threader.c threader.h
	@$(CC) -DTEST_CLASS -o threader.test $(CFLAGS) threader.c string.o array.o hash_master.o dictionary.o base.o mem_tracker.o -lpthread

threader.exe:	threader.c threader.h
	@$(CC) -DTEST_CLASS -o threader.exe $(CFLAGS) threader.c string.o array.o hash_master.o dictionary.o base.o mem_tracker.o /mingw/lib/libpthreadGC.a

mem_debugger.test:	mem_debugger.c mem_debugger.h
	@$(CC) -DTEST_CLASS -o mem_debugger.test $(CFLAGS) mem_debugger.c string.o array.o hash_master.o dictionary.o base.o -lpthread

md5.test:	md5.h md5.c
	@$(CC) -DTEST_CLASS -o md5.test $(CFLAGS) md5.c  base.o string.o  mem_tracker.o

bin_tree.test:	bin_tree.h bin_tree.c
	@$(CC) -DTEST_CLASS -o bin_tree.test $(CFLAGS) bin_tree.c  base.o string.o  mem_tracker.o

pop3.test:	pop3.h pop3.c
	@$(CC) -DTEST_CLASS -DDEBUG -o pop3.test $(CFLAGS) pop3.c  base.o string.o  mem_tracker.o inet_client.o stringlist.o list.o hash_master.o \

ftp_client.test:	ftp_client.h ftp_client.c
	@$(CC) -DTEST_CLASS -DDEBUG -o ftp_client.test $(CFLAGS) ftp_client.c  base.o string.o  mem_tracker.o inet_client.o stringlist.o list.o hash_master.o \
								   array.o

dbi.test:	dbi_manager.c
	$(CC) $(CFLAGS) -DTEST_CLASS -o dbi.test dbi_manager.c -ldl -pthread $(AFC_LIBS)

circular_list.test:	circular_list.h circular_list.c
	@$(CC) -DTEST_CLASS -DDEBUG -o circular_list.test $(CFLAGS) circular_list.c  base.o string.o  mem_tracker.o

btree.test:	btree.h btree.c
	@$(CC) -DTEST_CLASS -DDEBUG -o btree.test -Wall -g btree.c  base.o string.o  mem_tracker.o

fileops.test:	fileops.h fileops.c
	@$(CC) -DTEST_CLASS -DDEBUG -o fileops.test -Wall -g fileops.c  base.o string.o  mem_tracker.o

avl_tree.test:	avl_tree.h avl_tree.c
	@$(CC) -DTEST_CLASS -DDEBUG -o avl_tree.test -Wall -g avl_tree.c  base.o string.o  mem_tracker.o

tree.test: tree.h tree.c
	@$(CC) -DTEST_CLASS -DDEBUG -o tree.test -Wall -g tree.c base.o string.o mem_tracker.o

base64.test: base64.h base64.c
	@$(CC) -DTEST_CLASS -DDEBUG -o base64.test -Wall -g base64.c base.o string.o mem_tracker.o
