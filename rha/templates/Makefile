PREFIX=../..

CFLAGS=$(STD_CFLAGS) $(CFG_CFLAGS) -I$(PREFIX)/include -I.. -I.

TMPLDIR=$(PREFIX)/include/ctmpl
TMPLDEFS=-ialloc.h alloc=GC_malloc free=GC_free
MAKETMPL=$(PREFIX)/bin/maketmpl

TEMPLATES=tupleobject.h tuplesymbol.h tupleint.h \
	listint.h treeintint.h tuplevoidp.h \
	listobject.h hashsymbolobject.h listvoidp.h \
	treesymbolobject.h treestringint.h treeintstring.h \
	treeobjectobject.h treeintvoidp.h

OBJECTS=stringdefs.o

libtemplates.a: $(subst .h,.o,$(TEMPLATES)) $(OBJECTS)
	ar rcs $@ $^
	cp $(TEMPLATES) ..

templates : $(TEMPLATES)

clean:
	rm -f *.o

distclean:
	rm -f *.o
	rm -f .depend
	rm -f $(TEMPLATES) $(subst .h,.c,$(TEMPLATES)) libtemplates.a

depend:
	gcc -M $(CFLAGS) -DMAKE_DEPEND *.c >.depend

tupleobject.h tupleobject.c : $(TMPLDIR)/tuple.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=object_t -iobject.h $<

tuplesymbol.h tuplesymbol.c : $(TMPLDIR)/tuple.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=symbol_tr -isymbol_tr.h $<

## for parse_tr.c
tupleint.h tupleint.c : $(TMPLDIR)/tuple.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=int $<

## for parse_tr.c
tupletreeintint.h tupletreeintint.c : $(TMPLDIR)/tuple.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=treeintint_t -itreeintint.h $<

## for parse_tr.c
listint.h listint.c : $(TMPLDIR)/list.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=int $<

## for parse_tr.c
listvoidp.h listvoidp.c : $(TMPLDIR)/list.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=voidp -ivoidp_defs.h $<

## for parse_tr.c
tuplevoidp.h tuplevoidp.c : $(TMPLDIR)/tuple.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=voidp -ivoidp_defs.h $<

## for parse_tr.c
treeintint.h treeintint.c : $(TMPLDIR)/tree.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) K=int V=int -iintdefs.h lessthan=int_lessthan $<

## for parse_tr.c
treeintvoidp.h treeintvoidp.c : $(TMPLDIR)/tree.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) K=int V=voidp -iintdefs.h -ivoidp_defs.h lessthan=int_lessthan $<

listobject.h listobject.c : $(TMPLDIR)/list.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) T=object_t -iobject.h $<

hashsymbolobject.h hashsymbolobject.c : $(TMPLDIR)/hash.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) K=symbol_tr V=object_t -iobject.h -isymbol_tr.h compare=symbol_equal_symbol hash=symbol_hash $<

treesymbolobject.h treesymbolobject.c : $(TMPLDIR)/tree.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) K=symbol_tr V=object_t -iobject.h -isymbol_tr.h lessthan=symbol_less_symbol $<

treestringint.h treestringint.c : $(TMPLDIR)/tree.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) K=string_t V=int -istringdefs.h lessthan=string_lessthan $<

treeintstring.h treeintstring.c : $(TMPLDIR)/tree.tmpl Makefile
	$(MAKETMPL) $(TMPLDEFS) K=int V=string_t -iintdefs.h -istringdefs.h lessthan=int_lessthan $<

treeobjectobject.h treeobjectobject.c : $(TMPLDIR)/tree.tmpl Makefile objectdefs.h
	$(MAKETMPL) $(TMPLDEFS) K=object_t V=object_t -ituple_tr.h lessthan=tuple_less_tuple $<


ifeq ($(wildcard .depend),.depend)
  include .depend	
endif
