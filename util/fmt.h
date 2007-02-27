/*
 * fmt - user extensible formatted printing
 *
 * provides printf abilities which can be extended by registering
 * user-supplied formatters
 *
 * uses cstream
 *
 * (c) 2005 by Mikio L. Braun
 */

#ifndef FMT_H
#define FMT_H

#include <stdbool.h>
#include <limits.h>
#include <stdarg.h>

#include "cstream.h"

/* struct used by parsefmtspec() to collect C-style formatting
   options */
struct fmt_spec 
{
  bool flags[CHAR_MAX];  // flags found in format string
  int width;             // field width
  int prec;              // precision
};

/* function type for user-defined formatters 

   formatters are passed the output stream, the formatting string
   including the leading '%' and the actual type itself. Use
   parsefmtspec to parse C-style formatting options. */
typedef void fmt_f(STREAM *out,
		 char *spec,
		 va_list *ap);

/*
 * the basic printing functions
 */ 
/* print to cout */
extern void prtfmt(const char *fmt, ...);

/* print to a string of given size */
extern void snprtfmt(char *buffer, int len, const char *fmt, ...);

/* print to a buffer of variable size */
extern void sprtfmt(char **buffer, const char *fmt, ...);

/* print to a FILE */
extern void fprtfmt(FILE *f, const char *fmt, ...);

/* print to a STREAM */
extern void strprtfmt(STREAM *s, const char *fmt, ...);

/* print to a STREAM using va's */
extern void vprtfmt(STREAM *s, const char *fmt, va_list ap);


/*
 * user-supplied formatting functions
 */
/* register new formatter */
extern void addfmt(char c, fmt_f *f);

/* parse C-style formatting options */ 
extern void parsefmtspec(const char *fmt, struct fmt_spec *spec);

#endif
