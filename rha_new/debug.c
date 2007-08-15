#include "debug.h"

#include <stdio.h>
#include <stdarg.h>
#include "utils.h"

void _printdebug(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprint(stderr, fmt, &ap);
}

