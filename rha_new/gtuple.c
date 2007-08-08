/*
 * gtuple - a generic fixed length array of values
 *
 * (c) 2005 by Mikio L. Braun
 */

/*
 * This file has been generated from the gtuple.tmpl template + some
 * editing
 */

#include "gtuple.h"

#include <stdlib.h>
#include "utilalloc.h"

// initialize a gtuple with zero
void gtuple_init(gtuple_t *t, int len)
{
  t->len = len;
  t->array  = util_malloc(sizeof(intptr_t) * len);
  for(int i = 0; i < len; i++)
    t->array[i] = 0;
}

// initialize a gtuple with a given value
void gtuple_initto_(gtuple_t *t, int len, intptr_t x)
{
  t->len = len;
  t->array  = util_malloc(sizeof(intptr_t) * len);
  for(int i = 0; i < len; i++)
    t->array[i] = x;
}

// copy a gtuple
void gtuple_copy(gtuple_t *t, gtuple_t *t2)
{
  t->len = t2->len;
  t->array = util_malloc(sizeof(intptr_t) * t->len);
  for(int i = 0 ; i < t->len; i++)
    t->array[i] = t2->array[i];
}

// destroy a gtuple
void gtuple_clear(gtuple_t *t)
{
  util_free(t->array);
}

// execute a function for each element
void gtuple_foreach(gtuple_t *t, gtuple_proc *fct)
{
  for(int i = 0; i < t->len; i++)
    (*fct)(t->array[i]);
}


#ifdef USE_INLINES
#define INLINE inline
#else
#define INLINE
#endif
#include "gtuple_inline.c"

