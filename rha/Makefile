# make templates
# make depend
# make 
#

%.c : %.y

PREFIX=..

#
# standard symbols and auto-configuration (it's magic!!!!!)
#
export LD=gcc
export CC=gcc

export YACC=bison
export YFLAGS=-d -v


export STD_CFLAGS
export CFG_CFLAGS

### determine platform
PLATFORM=unix

# Mikio's laptop running windows/cygwin
ifeq (evo, $(HOSTNAME))
  PLATFORM=cygwin
endif

# Stefan's laptop running windows/cygwin
ifeq (xyan, $(HOSTNAME))
  PLATFORM=cygwin
endif

# Stefan's laptop running OS X (with Python 2.4 in fink)
ifeq (/Users/harmeli, $(HOME))
  PLATFORM=mac
endif

### PYTHON
export PY_CFLAGS PY_LDFLAGS PY_LOADLIBES

ifeq (evo, $(HOSTNAME))
  PY_CFLAGS=-I/cygdrive/c/Python24/include -D_WIN32
  PY_LDFLAGS=-L/cygdrive/c/Python24/libs
  PY_LOADLIBES=-lpython24
else
  PYVER=python2.4
  ifeq (mac, $(PLATFORM))
    PY_CFLAGS=-I/sw/include/$(PYVER)
    PY_LDFLAGS=-L/sw/lib/$(PYVER)/config
    PY_LOADLIBES=-l$(PYVER)
  else
    PY_CFLAGS=-I/usr/include/$(PYVER)
    PY_LDFLAGS=-L/usr/lib/$(PYVER)/config
    PY_LOADLIBES=-l$(PYVER)
  endif
endif

### ATLAS
export ATLAS_CFLAGS ATLAS_LDFLAGS ATLAS_LOADLIBES

ifeq (cygwin, $(PLATFORM))
  ATLAS_CFLAGS=-I$(HOME)/buildsite/ATLAS/include
  ATLAS_LDFLAGS=-L$(HOME)/buildsite/ATLAS/lib/Cygwin_PIIISSE1
  ATLAS_LOADLIBES=-llapack -lcblas -latlas
else 
  ifeq (mac, $(PLATFORM))
    ATLAS_CFLAGS=-I/System/Library/Frameworks/vecLib.framework/Versions/A/Headers
    ATLAS_LDFLAGS=-L/usr/local/lib
    ATLAS_LOADLIBES=-lcblas  -latlas
  else
    ATLAS_LDFLAGS=-L/usr/lib/sse2
    ATLAS_LOADLIBES=-llapack_atlas -lcblas  -latlas
  endif
endif

### MATLAB
export MATLAB_CFLAGS MATLAB_LDFLAGS MATLAB_LOADLIBES

ifeq (cygwin, $(PLATFORM))
  MATLAB_CFLAGS=-I/cygdrive/c/MATLAB6p1/extern/include/ -DMATLAB_R12
  MATLAB_LDFLAGS=-L/cygdrive/c/MATLAB6p1/extern/lib/win32/gnu
  MATLAB_LOADLIBES=-lmx -lmat -leng
else
  ifeq (mac, $(PLATFORM))
    MATLAB_HOME=/Applications/MATLAB7
#    MATLAB_CFLAGS=-I$(MATLAB_HOME)/extern/include -DMATLAB_R13
#    MATLAB_LDFLAGS=-L$(MATLAB_HOME)/extern/lib/mac
#    MATLAB_LOADLIBES=-lmx -lmat -leng
    MATLAB_CFLAGS=
    MATLAB_LDFLAGS=
    MATLAB_LOADLIBES=
  else
    MATLAB_HOME=/home/neuro/software/matlab6.5
    MATLAB_CFLAGS=-I$(MATLAB_HOME)/extern/include -DMATLAB_R13
    MATLAB_LDFLAGS=-L$(MATLAB_HOME)/extern/lib/glnx86
    MATLAB_LOADLIBES=-lmx -lmat -leng
  endif
endif


## Start collecting some flags
INCDIRS=-I$(PREFIX)/include -I/usr/X11R6/include -Itemplates -Istubs -I.
ifeq (mac, $(PLATFORM))
  STD_LDFLAGS=-L$(PREFIX)/lib -L/usr/X11R6/lib -Ltemplates -Lstubs
else
  STD_LDFLAGS=-L$(PREFIX)/lib -L/usr/X11R6/lib -L/usr/lib/w32api -Ltemplates \
          -Lstubs
endif
STD_LOADLIBES=-lm -lfl -lreadline -lgc -lutil -ltemplates -lstubs

STD_CFLAGS=-std=c99 -Wall -ggdb -pedantic -D_ANSI_SOURCE

##
## CHOOSE YOUR CONFIGURATION FROM THESE OPTIONS BELOW
## - or -
## set CFG_CFLAGS to whatever you want.
##

# standard configuration
#CFG_CFLAGS=-ggdb 

# std cfg with inlines
#CFG_CFLAGS= -DUSE_INLINES -finline

# release cfg
#CFG_CFLAGS=-DUSE_INLINES -O3 -DNDEBUG -finline

# profiling cfg 
#CFG_CFLAGS=-pg -O3

# profiling cfg with inlines
#CFG_CFLAGS=-pg -DUSE_INLINES -finline

##
## Construct the final set of flags
##

CFLAGS=$(STD_CFLAGS) $(CFG_CFLAGS) $(INCDIRS) $(PY_CFLAGS) $(ATLAS_CFLAGS) $(MATLAB_CFLAGS)
LDFLAGS=$(STD_LDFLAGS)  $(PY_LDFLAGS) $(ATLAS_LDFLAGS) $(MATLAB_LDFLAGS)
LOADLIBES=$(STD_LOADLIBES) $(PY_LOADLIBES)  $(ATLAS_LOADLIBES) $(MATLAB_LOADLIBES)

######################################################################
#
# VARIABLES
#
######################################################################


TR=plain_tr.o bool_tr.o int_tr.o real_tr.o complex_tr.o string_tr.o \
	matrix_tr.o list_tr.o tuple_tr.o symbol_tr.o \
	function_tr.o builtin_tr.o overloaded_tr.o method_tr.o \
	exception_tr.o parser_tr.o none_tr.o python_tr.o pyobject_tr.o \
	listit_tr.o thisproxy_tr.o dict_tr.o 

STUBS=$(subst _tr,_stub,$(TR))

OBJECTS=alloc.o object.o symtable.o primtype.o prule.o eval.o core.o \
        rhaparser.o rhalexer.o rhalexerpure.o messages.o rhavt.o \
	debug.o utils.o $(TR)

PROGRAMS=sexprha rhabarber

##############################################################
##
## TARGETS
##
##############################################################
rhabarber : rhabarber.o rhalexer.o rhaparser.o $(OBJECTS)

sexprha : sexprha.o sexplexer.o sexpparser.o $(OBJECTS)

rhaparser.c : rhaparser.tab.c
	mv $< $@

rhaparser.tab.h rhaparser.tab.c : rhaparser.y
	bison -d -v $<

rhalexerpure.o : rhalexerpure.fl rhalexerpure.h

rhalexer.o : rhalexer.fl rhalexer.h rhaparser.tab.h

rhaparser.y : rhalexer.h object.h core.h tuple_tr.h list_tr.h \
	real_tr.h int_tr.h string_tr.h

.PHONY: templates
templates :
	(cd templates; make)

objects : $(OBJECTS)

#------------------------------------------------------------
# administrative targets
#------------------------------------------------------------
depend : templates stubs
	(cd templates; make depend)
	(cd stubs; make depend)
	touch rha_stub.pl
	gcc -M $(CFLAGS) -DMAKE_DEPEND *.c >.depend

all : templates stubs rhabarber

clean : 
	rm -f *.o 
	rm -f rhalexerpure.c
	rm -f rhaparser.c

rmstubs :
	(cd stubs; make distclean)

rmtemplates :
	(cd templates; make distclean)

distclean : rmtemplates rmstubs clean
	rm -f $(PROGRAMS) .depend


##------------------------------------------------------------
## Auto-generated Files
##------------------------------------------------------------

##
## Stubs
##
#.PHONY: stubs
stubs : $(subst .o,.h,$(STUBS)) $(subst .o,.h,$(TR)) Makefile
	(cd stubs; make depend; make)

core_symbols.h : core_symbols
	./makesymbols.pl $<

plain_stub.h plain_stub.c : plain_tr.h rha_stub.pl
	./rha_stub.pl plain_tr.h
bool_stub.h bool_stub.c : bool_tr.h rha_stub.pl
	./rha_stub.pl bool_tr.h
none_stub.h none_stub.c : none_tr.h rha_stub.pl
	./rha_stub.pl none_tr.h
int_stub.h int_stub.c : int_tr.h rha_stub.pl
	./rha_stub.pl int_tr.h
real_stub.h real_stub.c : real_tr.h rha_stub.pl
	./rha_stub.pl real_tr.h
complex_stub.h complex_stub.c : complex_tr.h rha_stub.pl
	./rha_stub.pl complex_tr.h
string_stub.h string_stub.c : string_tr.h rha_stub.pl
	./rha_stub.pl string_tr.h
matrix_stub.h matrix_stub.c : matrix_tr.h rha_stub.pl
	./rha_stub.pl matrix_tr.h
list_stub.h list_stub.c : list_tr.h rha_stub.pl
	./rha_stub.pl list_tr.h
listit_stub.h listit_stub.c : list_tr.h rha_stub.pl
	./rha_stub.pl listit_tr.h
tuple_stub.h tuple_stub.c : tuple_tr.h rha_stub.pl
	./rha_stub.pl tuple_tr.h
symbol_stub.h symbol_stub.c : symbol_tr.h rha_stub.pl
	./rha_stub.pl symbol_tr.h
function_stub.h function_stub.c : function_tr.h rha_stub.pl
	./rha_stub.pl function_tr.h
builtin_stub.h builtin_stub.c : builtin_tr.h rha_stub.pl
	./rha_stub.pl builtin_tr.h
overloaded_stub.h overloaded_stub.c : overloaded_tr.h rha_stub.pl
	./rha_stub.pl overloaded_tr.h
method_stub.h method_stub.c : method_tr.h rha_stub.pl
	./rha_stub.pl method_tr.h
exception_stub.h exception_stub.c : exception_tr.h rha_stub.pl
	./rha_stub.pl exception_tr.h
parser_stub.h parser_stub.c : parser_tr.h rha_stub.pl
	./rha_stub.pl parser_tr.h
python_stub.h python_stub.c : python_tr.h rha_stub.pl
	./rha_stub.pl python_tr.h
pyobject_stub.h pyobject_stub.c : pyobject_tr.h rha_stub.pl
	./rha_stub.pl pyobject_tr.h
thisproxy_stub.h thisproxy_stub.c : thisproxy_tr.h rha_stub.pl
	./rha_stub.pl thisproxy_tr.h
dict_stub.h dict_stub.c : dict_tr.h rha_stub.pl
	./rha_stub.pl dict_tr.h
matlab_stub.h matlab_stub.c : matlab_tr.h rha_stub.pl
	./rha_stub.pl matlab_tr.h
mxarray_stub.h mxarray_stub.c : mxarray_tr.h rha_stub.pl
	./rha_stub.pl mxarray_tr.h
mxfunc_stub.h mxfunc_stub.c : mxfunc_tr.h rha_stub.pl
	./rha_stub.pl mxfunc_tr.h

messages.o : messages.c $(filter-out messages.c,$(wildcard *.c)) $(wildcard *.h)

rhalexerpure.c : rhalexerpure.fl
	flex -Pyypure -o$@ $<
%.c : %.fl
	flex -o$@ $<


ifeq ($(wildcard .depend),.depend)
  include .depend	
endif

