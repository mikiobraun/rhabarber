CC=gcc
CFLAGS=-std=c99 -I../ -ggdb -I/opt/local/include
LDFLAGS=-L/opt/local/lib
LOADLIBES=-lgc

SOURCES=$(wildcard *.c)
UTILOBJ=gtree.o utilalloc.o gtuple.o

TESTCASES=$(subst .c,,$(wildcard test_*.c))

default : run_tests

rha_types.h rha_init.c rha_init.h : rha_config.d rha_config.pl
	./rha_config.pl

test_symbol_fn : test_symbol_fn.o symbol_fn.o $(UTILOBJ)

test_tuple : test_tuple.o tuple_fn.o symbol_fn.o object.o symtable.o $(UTILOBJ)

test_eval : test_eval.o eval.o symbol_fn.o object.o symtable.o tuple_fn.o test_eval.o $(UTILOBJ)

test_symtable : test_symtable.o symtable.o object.o symbol_fn.o $(UTILOBJ) 

test_gtree : test_gtree.o $(UTILOBJ)

test_cases: $(TESTCASES)

run_tests: test_cases
	for i in $$(find . -name "test_*" -perm +a+x); do echo -e "---Test: $$i\n"; $$i; done

rha_lexer.c : rha_parser.tab.h
	flex rha_lexer.l

rha_parser_tab.h rha_parser_tab.c : rha_lexer.l rha_parser.y
	bison -d rha_parser.y

clean:
	rm -f *.o
	rm -f rha_types.h rha_init.h rha_init.c
	rm -f rha_parser_tab.h rha_parser_tab.c rha_lexer.c

depend:
	$(CC) $(CFLAGS) -M $(SOURCES) >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif