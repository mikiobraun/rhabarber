/*
 * dict - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#ifndef DICT_H
#define DICT_H

#include "primtype.h"
#include "object.h"

typedef struct dict_s *dict_tr;

extern primtype_t dict_type;

extern void dict_init(void);

extern constructor dict_tr dict_new();

extern method void     dict_insert(dict_tr d, object_t key, object_t val);
extern method object_t dict_search(dict_tr d, object_t key);
extern method int      dict_length(dict_tr d);

extern method string_t dict_to_string(dict_tr d);

#endif
