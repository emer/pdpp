########## ptest_gt makefile ############

GTDIR=/scgt
#GTDIR=../gtwork

# gt CFLAGS
GTCFLAGS=-DPLATFORM_UNIX -I$(GTDIR)/inc -L$(GTDIR)/linux-2.6/lib
USYSFLAGS=-I$(GTDIR)/apps/gtutils

# common CFLAGS
CFLAGS=$(GTCFLAGS) -Wall -O2 -fno-exceptions -fno-rtti

# common LIBFLAGS
LIBFLAGS=-lscgtapi -lpthread

CC=g++

all: ptest_gt

ptest_gt: ptest_gt.cc
	$(CC) -o $@ $(CFLAGS) $@.cc $(LIBFLAGS)

clean:
	rm -f ptest_gt
