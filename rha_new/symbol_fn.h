/*
 * symbol_fn - symbol functions
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#ifndef SYMBOL_FN_H
#define SYMBOL_FN_H

#include "rha_types.h"

extern _rha_ symbol_t symbol_new(string_t s);
extern _rha_ bool_t symbol_equal(symbol_t s, symbol_t t);
extern _rha_ string_t symbol_name(symbol_t s);

extern object_t symbol_wrap(symbol_t s);
extern object_t symbol_new_wrap(string_t s);
extern symbol_t symbol_unwrap(object_t s);

#endif
