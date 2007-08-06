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

#include "datatypes.h"

extern rhafun symbol_t symbol_new(string_t s);
extern rhafun bool_t symbol_equal(symbol_t s, symbol_t t);

#endif
