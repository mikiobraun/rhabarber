#include <stdarg.h>
#include <stdio.h>
#include "rha_types.h"

// this file should be compiled like this:
// gcc -dynamiclib -std=gnu99 -Wall -pedantic -fno-common -o example.so example.c

// e.g.
int f(int x) { 
  fprintf(stdout, "inside f\n");
  return 2*x; 
}

// call was
// ccode("result = f(arg1);", int, int)

void *wrapped_ccode(int narg, ...)
{
  va_list args;
  int result;
  union raw_t arg1;

  va_start(args, narg);
  arg1 = va_arg(args, union raw_t);
  result = f(arg1.i);
  return (void *) result;
}

