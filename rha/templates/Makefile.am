# Makefile.am for building all necessary templates
#
# (1) Run maketemplates.sh to generate all of the template files
# (2) compile those template files and the normal sources into libtemplates.a
#
# The hard thing was making this work with listing the set of
# TEMPLATES only once!  Using LIBADD with .o files did the trick
#

# the list of templates to be generated
TEMPLATESH=tupleobject.h tuplesymbol.h tupleint.h \
	listint.h treeintint.h tuplevoidp.h \
	listobject.h hashsymbolobject.h listvoidp.h \
	treesymbolobject.h treestringint.h treeintstring.h \
	treeobjectobject.h treeintvoidp.h

# templates with .h and .c files
TEMPLATES=$(TEMPLATESH) $(subst .h,.c,$(TEMPLATESH))

# this is the library we're going to built, which has two real source files
noinst_LIBRARIES = libtemplates.a
libtemplates_a_SOURCES = stringdefs.c stringdefs.h intdefs.h objectdefs.h \
	toodefs.h voidp_defs.h
# and many source files to be built automatically
libtemplates_a_LIBADD = $(subst .h,.$(OBJEXT),$(TEMPLATES))
BUILT_SOURCES = $(TEMPLATES)

EXTRA_DIST = maketemplates.sh

# don't forget to delete those files
CLEANFILES = $(TEMPLATES)

INCLUDES = -I@top_srcdir@/rha

$(TEMPLATES):
	. @srcdir@/maketemplates.sh

