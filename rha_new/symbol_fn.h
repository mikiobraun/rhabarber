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

extern  symbol_t symbol_new(string_t s);
extern  bool   symbol_equalequal(symbol_t s, symbol_t t);
extern  string_t symbol_name(symbol_t s);
extern  string_t symbol_to_string(symbol_t s);
extern  bool   symbol_valid(symbol_t s);

//extern any_t symbol_wrap(symbol_t s);
//extern any_t symbol_new_wrap(string_t s);
//extern symbol_t symbol_unwrap(any_t s);

#endif
