/*
 * dict_fn - dictionary functions
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#ifndef DICT_FN_H
#define DICT_FN_H

#include <stdarg.h>
#include "rha_types.h"

extern _rha_ dict_t   dict_new(void);
extern _rha_ int      dict_len(dict_t d);
extern _rha_ any_t    dict_get(dict_t d, intptr_t key);
extern _rha_ dict_t   dict_set(dict_t d, intptr_t key, any_t value);
extern _rha_ dict_t   dict_rm(dict_t d, intptr_t key);

extern _rha_ intptr_t dict_hash(any_t obj);  // generates keys

extern _rha_ list_t   dict_keys_to_list(dict_t d);
extern _rha_ list_t   dict_values_to_list(dict_t d);
extern _rha_ string_t dict_to_string(dict_t d);

#endif
