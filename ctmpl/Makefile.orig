CFLAGS=-std=c99 -ggdb -D_ANSI_SOURCE
LOADLIBES=-lfl
PROGRAMS=maketmpl

TEMPLATES=tuple.tmpl list.tmpl hash.tmpl stack.tmpl tree.tmpl
PREFIX=..

%.c : %.l
	flex -o$@ $<

default: maketmpl install

install:
	install $(TEMPLATES) $(PREFIX)/include/ctmpl
	install maketmpl $(PREFIX)/bin

clean : 
	rm -f *.o maketmpl

distclean : clean
	rm -f $(PROGRAMS) .depend

listint.c listint.h : list.tmpl
	maketmpl T=int $<

tupleint.c tupleint.h : tuple.tmpl
	maketmpl T=int -Istdlib.h $<

hashintint.c hashintint.h : hash.tmpl Makefile
	maketmpl K=int V=int -Istdlib.h -iintdefs.h compare=intcompare $<

hashstringint.c hashstringint.h : hash.tmpl stringdefs.h Makefile
	maketmpl K=string V=int -istringdefs.h compare=\!strcmp clonekey=strdup freekey=free hash=hashstring $<

testlist.c : listint.h

testlist : testlist.o listint.o

hashtest.c : hashintint.h hashstringint.h stringdefs.h

hashtest : hashtest.o hashstringint.o stringdefs.o hashintint.o

tupletest.c : tupleint.h listint.h

tupletest : tupletest.o tupleint.o listint.o

treestringint.c treestringint.h : tree.tmpl stringdefs.h Makefile
	maketmpl K=string V=int lessthan=stringlessthan -istringdefs.h tree.tmpl
treetest : treetest.o treestringint.o stringdefs.o