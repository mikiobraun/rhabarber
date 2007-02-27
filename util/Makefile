CC=gcc
AR=ar
CFLAGS=-std=c99 -Wall -pedantic -ggdb -D_ANSI_SOURCE

#OBJECTS=$(subst .c,.o,$(wildcard *.c))
OBJECTS=cstream.o fmt.o tictoc.o glist.o gtuple.o gtree.o utilalloc.o
HEADERS=$(subst .o,.h,$(OBJECTS))

default: install

libutil.a : $(OBJECTS)
	ar rcs $@ $^

install: libutil.a
	cp libutil.a ../lib/ 
	cp $(HEADERS) ../include

depend :
	gcc -M $(CFLAGS) -DMAKE_DEPEND *.c >.depend

clean:
	rm -f *.o
	rm -f *.a

distclean:
	rm -f *.o
	rm -f *.a
	rm -f ../lib/libutil.a

tuple_test: tuple_test.o gtuple.o utilalloc.o

list_test: list_test.o glist.o utilalloc.o

tree_test: tree_test.o gtree.o utilalloc.o 

gctest : gctest.o gc.o

cstreamtest: cstreamtest.o cstream.o

ifeq ($(wildcard .depend),.depend)
  include .depend	
endif
