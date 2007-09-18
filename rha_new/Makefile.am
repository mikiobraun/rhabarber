bin_PROGRAMS = rhabarber
rhabarber_SOURCES = alloc.c alloc.h \
	bool_fn.c bool_fn.h \
	core.c core.h \
	cstream.c cstream.h \
	debug.c debug.h \
	eval.c eval.h \
	excp.c excp.h \
	fmt.c fmt.h \
	glist.c glist.h \
	gtree.c gtree.h \
	gtuple.c gtuple.h \
	int_fn.c int_fn.h \
	list_fn.c list_fn.h \
	mat_fn.c mat_fn.h \
	messages.c messages.h \
	object.c object.h \
	parse.c parse.h \
	prules.c prules.h \
	python.c python.h \
	real_fn.c real_fn.h \
	rhabarber.c rha_config.d \
	rha_lexer.h rha_lexer.l \
	rha_parser.y string_fn.c \
	string_fn.h symbol_fn.c \
	symbol_fn.h symtable.c \
	symtable.h tuple_fn.c \
	tuple_fn.h utilalloc.c \
	utilalloc.h utils.c \
	utils.h


rhabarber_LDADD = @LEXLIB@
AM_CFLAGS = -I/opt/local/include -I/usr/include/python2.4 -Wall -pedantic
EXTRA_DIST = rha_config.pl prelude.rha test.rha

AM_YFLAGS = -d

BUILT_SOURCES = rha_types.h rha_init.h rha_init.c 
CLEANFILES = rha_types.h rha_init.h rha_init.c
rhabarber_LDADD += rha_init.o

rha_types.h rha_init.h rha_init.c : rha_config.d
	@srcdir@/rha_config.pl $<