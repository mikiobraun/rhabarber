CC=gcc
CFLAGS=-std=c99 -I../ -ggdb -I/opt/local/include
LDFLAGS=-L../util
LOADLIBES=-lutil -lgc

SOURCES=$(wildcard *.c)
TESTCASES=$(subst .c,,$(wildcard test_*.c))

rha_types.h rha_init.c rha_init.h : rha_config.d rha_config.pl
	./rha_config.pl

test_symbol_fn.o : test_symbol_fn.c symbol_fn.h minunit.h

test_symbol_fn : test_symbol_fn.o symbol_fn.o

# tuple
test_tuple_fn.o : test_tuple_fn.c test_tuple_fn.h minunit.h symbol_fn.h object.h

test_tuple : test_tuple.o tuple_fn.o symbol_fn.o object.o symtable.o

# eval
test_eval.o : test_eval.c eval.h symbol_fn.h rha_types.h

test_eval : test_eval.o eval.o symbol_fn.o object.o symtable.o tuple_fn.o test_eval.o

# symtable
test_symtable.o : test_symtable.c object.h symbol_fn.h

test_symtable : test_symtable.o symtable.o object.o symbol_fn.o

testcases: $(TESTCASES)

run_tests: testcases
	for i in $$(find . -name "test_*" -perm +a+x); do echo -e "---Test: $$i\n"; $$i; done

clean:
	rm -f *.o
	rm rha_types.h rha_init.h rha_init.c

depend:
	gcc -E $(CFLAGS) x$(SOURCES) >.depend