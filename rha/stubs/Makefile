PREFIX=../..

LD=gcc
CC=gcc

INCDIRS=-I$(PREFIX)/include -I/usr/X11R6/include -I.. -I. -I../templates  

CFLAGS=$(STD_CFLAGS) $(CFG_CFLAGS) $(INCDIRS) $(PY_CFLAGS) $(MATLAB_CFLAGS)

#rlease flags
#CFLAGS=-std=c99 $(INCDIRS) -Wall -pedantic -DUSE_INLINES -O3 -DNDEBUG

OBJECTS=$(subst .c,.o,$(wildcard *.c))

libstubs.a : $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -f *.o

distclean:
	rm -f *.o *_stub.[ch]
	rm -f .depend
	rm -f $(OBJECTS) libstubs.a

depend:
	gcc -M $(CFLAGS) -DMAKE_DEPEND *.c >.depend


ifeq ($(wildcard .depend),.depend)
  include .depend	
endif
