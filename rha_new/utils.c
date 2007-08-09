#include "utils.h"

#include <string.h>
#include "fmt.h"
#include "alloc.h"
#include "object.h"

#ifdef _NOT_YET__
#include "symbol_fn.h"

/* print an object */
static void fmt_object(STREAM *s, char *fmt, va_list ap)
{
  if (fmt[strlen(fmt) - 2] != '#')
    strputs(s, object_to_string(va_arg(ap, object_t)));
  else
    strputs(s, rha_lookup(va_arg(ap, object_t), symbol_new("typename")));
}
#endif

void print(const char *fmt, ...)
{
#ifdef _NOT_YET_
  addfmt('o', fmt_object);
#endif
  va_list ap;
  va_start(ap, fmt);
  vprtfmt(cerr, fmt, ap);
  va_end(ap);
}

void vprint(const char *fmt, va_list ap)
{
#ifdef _NOT_YET_
  addfmt('o', fmt_object);
#endif
  vprtfmt(cout, fmt, ap);
}

void fprint(FILE *f, const char *fmt, ...)
{
#ifdef _NOT_YET_
  addfmt('o', fmt_object);
#endif
  STREAM *s = fstropen(f);
  va_list ap;
  va_start(ap, fmt);
  vprtfmt(s, fmt, ap);
  va_end(ap);
  strclose(s);
}

void vfprint(FILE * f, const char *fmt, va_list ap)
{
#ifdef _NOT_YET_
  addfmt('o', fmt_object);
#endif
  STREAM *s = fstropen(f);
  vprtfmt(s, fmt, ap);
  strclose(s);
}

char *sprint(const char *fmt, ...)
{
  char *retval;
#ifdef _NOT_YET_
  addfmt('o', fmt_object);
#endif
  STREAM *str = sstropen(NULL, 0);

  va_list ap;
  va_start(ap, fmt);
  vprtfmt(str, fmt, ap);
  va_end(ap);

  retval = gc_strdup(sstrbuffer(str));
  
  strclose(str);

  return retval;
}

char *vsprint(const char *fmt, va_list ap)
{
  char *retval;
#ifdef _NOT_YET_
  addfmt('o', fmt_object);
#endif
  STREAM *str = sstropen(NULL, 0);

  vprtfmt(str, fmt, ap);

  retval = gc_strdup(sstrbuffer(str));
  
  strclose(str);

  return retval;
}


void printusage(char *str)
{
  printf("usage: %s\n", str);
}
