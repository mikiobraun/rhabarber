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
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# todo: write sensible checks for this!

#
# Checking for python!
#
AM_PATH_PYTHON([2.3],,[:])
#AC_CHECK_HEADERS([python$PYTHON_VERSION/Python.h],[CFLAGS='CFLAGS -I)
AM_CONDITIONAL([HAVE_PYTHON],[test x"$PYTHON" = x":"])

#
# Checking for matlab!
#
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

# these are taken from autoscan
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h libintl.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memset sqrt strchr strdup])
# end of functions taken from autoscan

AM_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile
		util/Makefile 
		ctmpl/Makefile 
		rha/Makefile 
		rha/templates/Makefile rha/templates/maketemplates.sh])
AC_OUTPUT

