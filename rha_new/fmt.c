#include "fmt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define ALLOC(p) p = malloc(sizeof(*p))

static bool fmt_initialized = false;
static fmt_f *table[256];

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define HAVE_STRDUP
#ifndef HAVE_STRDUP
static char *strdup(const char *s);
#endif

static void fmt_init();


/*
 * The main working horse
 */

void vprtfmt(struct stream *s, const char *fmt, va_list *ap)
{
  if (!fmt_initialized) fmt_init();

  char *f = strdup(fmt);

  char *start = 0;

  /* iterate over all of the string. "start" functions both as a mode
     flag and as a pointer to the beginning of the argument string.

     When start == 0, all characters except for '%' are copied to
     output unaltered. '%' sets start to the current character and
     starts a format string.

     When start != 0, the loop does not output strings but instead
     waits for an alpha. Then, the format string and the current
     va_list is passed to the format function found int fmttable.
   */
  for(char *p = f; *p; p++) {
    if(!start) {
      if(*p == '%')  start = p;
      else           strputc(s, *p);
    }
    else {
      if(isalpha(*p) &&
	 strchr("hljztL", *p) == NULL ) {
	char saved_char = p[1];
	p[1] = '\0';

	//fprintf(stderr, "[%s]", start);

	fmt_f *f = table[(int)*p];

	if(f != NULL)  f(s, start, ap);
	else   
	  for(char *q = start; *q; q++)
	    strputc(s, *q);

	start = 0;
	p[1] = saved_char;
      }
      else if (*p == '%') {
	start = 0;
	strputc(s, '%');
      }
    }
  }

  free(f);
}


/*
 * Some convenience functions
 */ 

void prtfmt(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vprtfmt(cout, fmt, &ap);
  va_end(ap);
}


void fprtfmt(FILE *f, const char *fmt, ...)
{
  STREAM *s = fstropen(f);

  va_list ap;
  va_start(ap, fmt);
  vprtfmt(s, fmt, &ap);
  va_end(ap);

  strclose(s);
}


void snprtfmt(char *buffer, int len, const char *fmt, ...)
{
  STREAM *s = sstropen(buffer, len);

  va_list ap;
  va_start(ap, fmt);
  vprtfmt(s, fmt, &ap);
  va_end(ap);

  strclose(s);
}


void sprtfmt(char **buffer, const char *fmt, ...)
{
  STREAM *s = sstropen(NULL, 0);

  va_list ap;
  va_start(ap, fmt);
  vprtfmt(s, fmt, &ap);
  va_end(ap);
  
  *buffer = strdup(sstrbuffer(s));

  strclose(s);
}


/*
 * format functions
 */

#define STDFMTBUFSIZE 1024 // maximum format size for standard formats
static char buffer[STDFMTBUFSIZE];

void fmt_int(STREAM *s, char *fmt, va_list *ap)
{
  int l = strlen(fmt);
  if (fmt[l - 2] == 'l')
    if (l > 3 && fmt[l - 3] == 'l')
      snprintf(buffer, STDFMTBUFSIZE, fmt, (long long)va_arg(*ap, long long));
    else
      snprintf(buffer, STDFMTBUFSIZE, fmt, (long)va_arg(*ap, long));      
  else
    snprintf(buffer, STDFMTBUFSIZE, fmt, va_arg(*ap, int));
  strputs(s, buffer);
}


void fmt_char(STREAM *s, char *fmt, va_list *ap)
{
  snprintf(buffer, STDFMTBUFSIZE, fmt, va_arg(*ap, int));
  strputs(s, buffer);
}


void fmt_uint(STREAM *s, char *fmt, va_list *ap)
{
  int l = strlen(fmt);
  if (fmt[l - 2] == 'l')
    if (l > 3 && fmt[l - 3] == 'l')
      snprintf(buffer, STDFMTBUFSIZE, fmt, (unsigned long long)va_arg(*ap, unsigned long long));
    else
      snprintf(buffer, STDFMTBUFSIZE, fmt, (unsigned long)va_arg(*ap, unsigned long));      
  else
    snprintf(buffer, STDFMTBUFSIZE, fmt, (unsigned int)va_arg(*ap, unsigned int));
  strputs(s, buffer);
}

void fmt_double(STREAM *s, char *fmt, va_list *ap)
{
  if (fmt[strlen(fmt) - 2] == 'L') {
    snprintf(buffer, STDFMTBUFSIZE, fmt, (long double)va_arg(*ap, long double));
  }
  else
    snprintf(buffer, STDFMTBUFSIZE, fmt, va_arg(*ap, double));
  strputs(s, buffer);
}


void fmt_string(STREAM *s, char *fmt, va_list *ap)
{
  snprintf(buffer, STDFMTBUFSIZE, fmt, va_arg(*ap, char*));
  strputs(s, buffer);
}

void fmt_voidptr(STREAM *s, char *fmt, va_list *ap)
{
  snprintf(buffer, STDFMTBUFSIZE, fmt, va_arg(*ap, void*));
  strputs(s, buffer);
}

static 
void fmt_init()
{
  for(int i = 0; i < CHAR_MAX; i++)
    table[i] = NULL;

  /* actually, we're not done yet, but we are also called from addfmt,
     therefore, we have to set it right now */
  fmt_initialized = true;

  addfmt('d', fmt_int);
  addfmt('c', fmt_char);
  addfmt('u', fmt_uint);
  addfmt('x', fmt_uint);
  addfmt('X', fmt_uint);
  addfmt('f', fmt_double);
  addfmt('f', fmt_double);
  addfmt('E', fmt_double);
  addfmt('E', fmt_double);
  addfmt('g', fmt_double);
  addfmt('G', fmt_double);
  addfmt('a', fmt_double);
  addfmt('A', fmt_double);
  addfmt('s', fmt_string);
  addfmt('p', fmt_voidptr);
}


void addfmt(char c, fmt_f f)
{
  if (!fmt_initialized) fmt_init();
  table[(int)c] = f;
}

/*
 * helper functions
 */

#ifndef HAVE_STRDUP
static
char *strdup(const char *s)
{
  char *p = malloc(sizeof(char) * (strlen(s) + 1));
  strcpy(p, s);
  return p;
}
#endif

/* parse a format string. This basically consists in collecting all
   non-numerical characters into the flag array and extracting two
   numbers, one before the point and one behind (if existing) */
void parsefmtspec(const char *fmt, struct fmt_spec *spec)
{
  char *f = strdup(fmt + 1);
  f[strlen(f) - 1] = '\0';

  char *first = 0;
  char *point = 0;
  char *last = 0;

  // clear all flags
  for (int i = 0; i < CHAR_MAX; i++)
    spec->flags[i] = false;

  // collect the flags and find occurrence of first numerical
  // character, then tha last, and then a (possible decimal point
  // inbetween). Then extract field and width.
  for (char *p = f; *p; p++) {
    if (isdigit(*p)) {
      if(!first) first = p;
      last = p;
    }
    else if (*p == '.') {
      point = p;
      if (!first) { // ups, no digits left of point?
	first = point;
	last = point;
      }
    }
    else {
      spec->flags[(int)*p] = true;
    }
  }

  if (first) {
    if (!point) {
      last[1] = '\0';
      spec->width = atoi(first);
      spec->prec = 0;
    }
    else {
      point[0] = '\0';
      last[1] = '\0';
      spec->width = atoi(first);
      spec->prec = atoi(point + 1);
    }
  }
  else {
    spec->width = 0;
    spec->prec = 0;
  }

  free(f);
}
