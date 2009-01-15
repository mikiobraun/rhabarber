/*
 * string_fn - functions for dealing with strings
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#ifndef STRING_FN_H
#define STRING_FN_H

#include "rha_types.h"

extern  string_t string_copy(string_t other);
extern  string_t string_concat(string_t s1, string_t s2);
extern  int    string_len(string_t s);
extern  bool   string_equalequal(string_t s1, string_t s2);
extern  string_t string_get(string_t s, int i);
extern  string_t string_set(string_t s, int i, string_t v);

extern  string_t string_to_string(string_t s);
#endif
