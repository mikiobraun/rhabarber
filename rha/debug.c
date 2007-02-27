#include "debug.h"

#include <stdio.h>
#include <stdarg.h>
#include "utils.h"
#include "object.h"

void _printdebug(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprint(stderr, fmt, ap);
}

/*
 * These two functions are useful for debugging.
 * They have rather short names, therefore they are
 * static.
 * The dummy_function below calls these to make
 * gcc shut up.
 */

static
void obj(object_t o)
{
  print("%o\n", o);
}

static 
void stk(object_t o)
{
  print("%o\n", o);
  while ( (o = object_parent(o)) ) {
    print("%o\n", o);
  }
}


void dummy_function_to_call_obj_and_stk()
{
  obj(0); stk(0);
}
