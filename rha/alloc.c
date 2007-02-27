#include "alloc.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char *gc_strdup(const char *c)
{
  char *p = GC_MALLOC(strlen(c) + 1);
  strcpy(p, c);
  return p;
}

char *nongc_strdup(const char *c)
{
  char *p = malloc(strlen(c) + 1);
  strcpy(p, c);
  return p;
}

