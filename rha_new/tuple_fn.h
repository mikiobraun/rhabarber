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

#include "datatypes.h"

extern rhafun object_t tuple_new(int_t len);
extern rhafun int_t    tuple_len(tuple_t t);
extern rhafun object_t tuple_get(tuple_t t, int_t i);
extern rhafun void_t   tuple_set(tuple_t t, int_t i, object_t s);

#endif
