/*
 * tuple_fn - tuple functions
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#include "tuple_fn.h"
#include <stdarg.h>
#include "alloc.h"
#include "list_fn.h"
#include "messages.h"
#include "gtuple.h"

tuple_t tuple_new(int_t len)
{
  tuple_t t = ALLOC_SIZE(sizeof(struct gtuple));
  gtuple_init(t, len);
  return t;
}

int_t tuple_len(tuple_t t)
{
  return gtuple_length(t);
}

object_t tuple_get(tuple_t t, int_t i)
{
  if (i<0 || i>=gtuple_length(t))
    rha_error("(tuple) index out-of-bounds");
  return gtuple_getp(t, i);
}

tuple_t tuple_set(tuple_t t, int_t i, object_t s)
{
  if (i<0 || i>=gtuple_length(t))
    rha_error("(tuple) index out-of-bounds");
  gtuple_set(t, i, s);
  return t;
}

tuple_t tuple_make(int_t narg, ...)
{
  va_list ap;
  va_start(ap, narg);
  tuple_t args = tuple_new(narg);
  for (int i = 0; i < narg; i++)
    tuple_set(args, i, va_arg(ap, object_t));
  va_end(ap);

  return vtuple_make(args);
}

tuple_t vtuple_make(tuple_t args)
{
  return args;
}

list_t tuple_to_list(tuple_t t)
{
  list_t l = list_new();
  int tlen = tuple_len(t);
  for (int i=0; i<tlen; i++)
    list_append(l, tuple_get(t, i));
  return l;
}
