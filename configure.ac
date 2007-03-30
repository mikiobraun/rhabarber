# rhabarber
#
# (c) 2005-2007 Mikio L. Braun, and Stefan Harmeling
#
#               mikiobraun@gmail.com, harmeling@gmail.com
#
# Code hosted at http://code.google.com/p/rhabarber
#
AC_PREREQ([2.61])
AC_INIT([rhabarber], [0.1], [mikiobraun@gmail.com])
AC_CONFIG_AUX_DIR([config])

AM_INIT_AUTOMAKE([1.10 foreign])

# this works okay, but configure does not find the files anymore... ?!
AC_PROG_INSTALL

# we're not so nice about this. We WANT a c99 compliant compiler!
AC_PROG_CC
AC_PROG_CC_C99
if test "$ac_cv_prog_cc_c99" = no; then
   AC_MSG_ERROR([Sorry, this package requires a c99 able compiler!])
fi

# since we're building some static libraries
AC_PROG_RANLIB

# some more programs
AC_PROG_YACC
AM_PROG_LEX

# todo: write sensible checks for this!
AM_CONDITIONAL([HAVE_PYTHON],[false])
AM_CONDITIONAL([HAVE_MATLAB],[false])

# check for libraries
AC_CHECK_LIB([gc],[GC_malloc])
if test "$ac_cv_lib_gc_GC_malloc" = no; then
   AC_MSG_ERROR([Sorry, but I definitely need lib gc])
fi 

# check for library functions
AC_CHECK_FUNCS([strdup])
AC_CHECK_LIB([readline], [readline])
if test "$ac_cv_lib_readline_readline" = no; then
   AC_MSG_ERROR([Sorry, but we like readline so much, that we just have to have it!])
fi
AC_CHECK_LIB([cblas],[cblas_dgemm],[],[],[-lm])
AC_CHECK_LIB([m],[sin])

AM_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile
		util/Makefile 
		ctmpl/Makefile 
		rha/Makefile 
		rha/templates/Makefile])
AC_OUTPUT

