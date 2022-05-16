#
#
#
ifeq ($(DEFINE),MINGW)

CFLAGS=-Wall -O2 -DSTATIC

else

CFLAGS=-Wall -O2 -fPIC -Wno-unused-but-set-variable

endif

CC=gcc
AR=ar

SRC=config.h get.c internal.h maketables.c pcre.c pcre.h study.c
OBJS=get.o maketables.o pcre.o study.o

all:	libpcre.a pcregrep
	
mingw_all:	libpcre.a 

libpcre.a:	$(OBJS)
	@$(AR) rcs libpcre.a $(OBJS)

%.o : %.c 
	@echo "Building $@"
	@$(CC) -c $(CFLAGS) $< -o $@

pcregrep:	pcregrep.c libpcre.a
	@$(CC) $(CFLAGS) -o pcregrep pcregrep.c libpcre.a

clean:
	@rm -f *.o
	@rm -f *.a
	@rm -f pcregrep
