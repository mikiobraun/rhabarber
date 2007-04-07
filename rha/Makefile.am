# rha/Makefile.am
#
SUBDIRS = templates

bin_PROGRAMS = rhabarber
rhabarber_SOURCES = rhabarber.c \
	rhalexer.l rhalexer.h rhalexerpure.h \
	rhaparser.y rhaparser_extra.h \
	alloc.c alloc.h \
	object.c object.h \
	symtable.c symtable.h \
	primtype.c primtype.h \
	prule.c prule.h \
	eval.c eval.h \
	core.c core.h \
	messages.c messages.h \
	rhavt.c rhavt.h \
	debug.c debug.h \
	utils.c utils.h \
	plain_tr.c plain_tr.h \
	bool_tr.c bool_tr.h \
	int_tr.c int_tr.h \
	real_tr.c real_tr.h \
        complex_tr.c complex_tr.h \
	string_tr.c string_tr.h \
	matrix_tr.c matrix_tr.h \
	list_tr.c list_tr.h \
	tuple_tr.c tuple_tr.h \
	symbol_tr.c symbol_tr.h	\
	function_tr.c function_tr.h \
	builtin_tr.c builtin_tr.h \
	overloaded_tr.c overloaded_tr.h \
	method_tr.c method_tr.h \
	exception_tr.c exception_tr.h \
	parser_tr.c parser_tr.h \
	none_tr.c none_tr.h \
	listit_tr.c listit_tr.h \
	thisproxy_tr.c thisproxy_tr.h \
	dict_tr.c dict_tr.h
EXTRA_rhabarber_SOURCES = list_inline.c matrix_inline.c object_inline.c \
	symbol_inline.c tuple_inline.c rhalexerpure.l \
	tree_tr.h
rhabarber_LDADD = templates/libtemplates.a @LEXLIB@ @top_srcdir@/util/libutil.a
AM_CFLAGS = -I@top_srcdir@/util -I@srcdir@/templates
EXTRA_DIST = rha_stub.pl makesymbols.pl test.rha init.rha

AM_YFLAGS = -d

BUILT_SOURCES = rhalexerpure.c rhaparser.h 
CLEANFILES = rhalexerpure.c
rhabarber_LDADD += rhalexerpure.o

rhalexerpure.c : rhalexerpure.l
	@LEX@ -Pyypure -o$@ $<

#----------------------------------------------------------------------
# let's see if we want to have python or matlab...
if HAVE_PYTHON
rhabarber_SOURCES += python_tr.c python_tr.h pyobject_tr.c pyobject_tr.h
AM_CFLAGS += -DHAVE_PYTHON
endif

if HAVE_MATLAB
rhabarber_SOURCES += matlab_tr.c matlab_tr.h mxarray_tr.c mxarray_tr.h	\
	mxfunc_tr.c mxfunc_tr.h
AM_CFLAGS += -DHAVE_MATLAB
endif

#----------------------------------------------------------------------
# and here's where the magic start... We search for actual files of
# the form "*_tr.c", massage the output a bit, and get that for every
# "*_tr.c", a "*_stub.o" has to be linked.
STUBS=$(subst _tr.c,_stub.o,$(filter %_tr.c,$(rhabarber_SOURCES)))
rhabarber_LDADD += $(STUBS)

# some more stuff so that this actually gets built, and cleaned
BUILT_SOURCES += $(subst .o,.c,$(STUBS)) $(subst .o,.h,$(STUBS))
CLEANFILES += $(subst .o,.c,$(STUBS)) $(subst .o,.h,$(STUBS))

# and this is how you do it!
%_stub.h %_stub.c : %_tr.h
	@srcdir@/rha_stub.pl $<

#----------------------------------------------------------------------
# the core_symbols.h file
EXTRA_rhabarber_SOURCES += core_symbols
BUILT_SOURCES += core_symbols.h
CLEANFILES += core_symbols.h

core_symbols.h : core_symbols
	@srcdir@/makesymbols.pl @srcdir@/core_symbols

#rhaparser.c : rhaparser.tab.c
#	mv $< $@