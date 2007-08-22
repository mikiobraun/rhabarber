CC=gcc
CFLAGS=-std=c99 -I../ -ggdb -I/opt/local/include -Wall -pedantic
#CFLAGS=-std=c99 -I../ -O3 -I/opt/local/include -Wall -pedantic

LDFLAGS=-L/opt/local/lib
LOADLIBES=-lgc -lreadline

ALLSOURCES=$(wildcard *.c)
SOURCES=alloc.o eval.o core.o \
	glist.o gtree.o gtuple.o list_fn.o messages.o object.o \
	parse.o rha_init.o rha_lexer.o rha_parser.tab.o symbol_fn.o \
	symtable.o tuple_fn.o utils.o utilalloc.o fmt.o cstream.o \
	int_fn.o prules.o string_fn.o debug.o excp.o
UTILOBJ=gtree.o utilalloc.o gtuple.o

TESTCASES=$(subst .c,,$(wildcard test_*.c))

######################################################################
#
# ABSTRACT TARGETS
#
#

#
# the default target
#
default : run_tests

run_tests: test_cases
	for i in $$(find . -name "test_*" -perm +a+x); do echo -e "---Test: $$i\n"; $$i; done
#
# administrative targets
#
clean:
	rm -f *.o
	rm -f rha_types.h rha_init.h rha_init.c
	rm -f rha_parser_tab.h rha_parser_tab.c rha_lexer.c

depend: rha_types.h
	$(CC) $(CFLAGS) -M $(ALLSOURCES) rha_lexer.l rha_parser.y >.depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

TAGS: $(ALLSOURCES)
	etags $^

######################################################################
#
# AUTOMATIC FILE GENERATIONS
#

#
# rule for stubs from rha_config.d
#
rha_types.h rha_init.c rha_init.h : rha_config.d rha_config.pl object.h eval.h parse.h core.h int_fn.h
	./rha_config.pl

rha_lexer.c : rha_parser_tab.h
	flex -o$@ rha_lexer.l

rha_parser_tab.h rha_parser_tab.c : rha_lexer.l rha_parser.y rha_types.h
	bison -d rha_parser.y

######################################################################
#
# PROGRAMS
#

#
# le programme main
#
rhabarber: $(SOURCES)

#
# tests
#
test_symbol_fn : test_symbol_fn.o symbol_fn.o $(UTILOBJ)

test_tuple : test_tuple.o tuple_fn.o symbol_fn.o object.o symtable.o $(UTILOBJ)

test_eval : test_eval.o eval.o symbol_fn.o object.o symtable.o tuple_fn.o test_eval.o $(UTILOBJ)

test_symtable : test_symtable.o symtable.o object.o symbol_fn.o $(UTILOBJ) 

test_gtree : test_gtree.o $(UTILOBJ)

test_rha_lexer : test_rha_lexer.o rha_lexer.o object.o symbol_fn.o symtable.o $(UTILOBJ) 

test_cases: $(TESTCASES)

test_fmt : test_fmt.o fmt.o cstream.o