/*
 * exceptions for C
 *
 * Provides some rudimentary exceptions facilities for C.
 * 
 * (c) 2005 by Mikio L. Braun
 */
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "../rha/object.h"

#ifndef EXCEPT_H
#define EXCEPT_H

/* maximum number of nested try blocks (assertion raised if violated) */
#define EXCEPT_MAX_NESTING  16

/* you do not manipulate these directly */
extern jmp_buf except_stack[EXCEPT_MAX_NESTING];
extern object_t except_retval[EXCEPT_MAX_NESTING];
extern int except_tos;

/*
 * The macros are used as follows:
 *
 *      try
 *         code
 *      catch
 *         code
 *      endtry;
 *
 * Exceptions are raised by "throw(e)", where e can be an int or a
 * pointer. To access the exception itself, use the "exception" macro
 * in the catch block.
 *
 * For example:
 *      try
 *         throw(-1);
 *      catch
 *         if (exception == -1)
 *           printf("I caught the exception!\n");
 *      endtry
 */

// NOTE: inside the 'try' and 'catch' there shall not be an 'return'
// statement.  Otherwise, 'except_tos' is not properly handled.

#define try    do { \
  assert(except_tos < EXCEPT_MAX_NESTING); \
  except_tos++; \
  int _exceptval = setjmp(except_stack[except_tos]); \
  if ( !_exceptval ) 

#define catch     else

#define throw(e)  do { \
  if (except_tos < 0) { \
    fprintf(stderr, "function %s, %s:%d: uncaught exception\n", \
      __func__, __FILE__, __LINE__); \
    abort(); \
  } \
  except_retval[except_tos] = (e); \
  longjmp(except_stack[except_tos], 1); \
  } while (0)

#define endtry except_tos--; } while(0);

#define exception (except_retval[except_tos])

#endif
