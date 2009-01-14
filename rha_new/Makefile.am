bin_PROGRAMS = rhabarber
rhabarber_SOURCES = alloc.c alloc.h \
	bool_fn.c bool_fn.h \
	ccode.c ccode.h \
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
	python_fn.c python_fn.h \
	real_fn.c real_fn.h \
	rhabarber.c \
	rha_lexer.h rha_lexer.l \
	rha_parser.y rha_parser_fcts.h string_fn.c \
	string_fn.h symbol_fn.c \
	symbol_fn.h symtable.c \
	symtable.h tuple_fn.c \
	tuple_fn.h utilalloc.c \
	utilalloc.h utils.c \
	utils.h

rhabarber_LDADD = @LEXLIB@
AM_CFLAGS = -Wall -pedantic -fno-common
EXTRA_DIST = rha_config.pl prelude.rha test.rha rha_config.d

AM_YFLAGS = -d

BUILT_SOURCES = rha_types.h rha_init.h rha_init.c
CLEANFILES = rha_types.h rha_init.h rha_init.c
rhabarber_LDADD += rha_init.o
rhabarber_LDFLAGS = -framework Python

rha_types.h rha_init.h rha_init.c : rha_config.d rha_config.pl $(rhabarber_SOURCES)
	@srcdir@/rha_config.pl
