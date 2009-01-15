/*
 * tuple_fn - tuple functions
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#ifndef TUPLE_FN_H
#define TUPLE_FN_H

#include <stdarg.h>
#include "rha_types.h"

extern  tuple_t  tuple_new(int len);
extern  int    tuple_len(tuple_t t);
extern  any_t    tuple_get(tuple_t t, int i);
extern  tuple_t  tuple_set(tuple_t t, int i, any_t s);

extern  list_t   tuple_to_list(tuple_t t);
extern  string_t tuple_to_string(tuple_t t);

extern  tuple_t  tuple_make(int narg, ...);
extern  tuple_t  vtuple_make(tuple_t args);

extern  tuple_t  tuple_shift(tuple_t t);

#endif
