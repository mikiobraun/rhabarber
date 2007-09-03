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
#include "utils.h"
#include "string_fn.h"

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

any_t tuple_get(tuple_t t, int_t i)
{
  int tlen = gtuple_length(t);
  if (i >= tlen || -i > tlen)
    rha_error("index out-of-bounds");
  if (i < 0) i = tlen + i;
  return gtuple_getp(t, i);
}

tuple_t tuple_set(tuple_t t, int_t i, any_t s)
{
  int tlen = gtuple_length(t);
  if (i >= tlen || -i > tlen)
    rha_error("index out-of-bounds");
  if (i < 0) i = tlen + i;
  gtuple_set(t, i, s);
  return t;
}

tuple_t tuple_make(int_t narg, ...)
{
  va_list ap;
  va_start(ap, narg);
  tuple_t args = tuple_new(narg);
  for (int i = 0; i < narg; i++)
    tuple_set(args, i, va_arg(ap, any_t));
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

string_t tuple_to_string(tuple_t t)
{
  string_t s = sprint("(");
  int tlen = tuple_len(t);
  for(int i = 0; i < tlen; i++) {
    if (i > 0) s = string_concat(s, " ");
    s = string_concat(s, to_string(tuple_get(t, i)));
  }
  return string_concat(s, ")");
}

tuple_t tuple_shift(tuple_t in)
{
  int l = tuple_len(in);
  tuple_t out = tuple_new(l - 1);
  for(int i = 1; i < l; i++)
    tuple_set(out, i - 1, tuple_get(in , i));
  return out;

}
