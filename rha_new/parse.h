#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ object_t parse(string_t);

extern _rha_ object_t add_keyword(string_t, int_t);

extern _rha_ object_t add_macro_prule(string_t, object_t);
extern _rha_ object_t add_postfix_prule(string_t, object_t);
extern _rha_ object_t add_prefix_prule(string_t, object_t);
extern _rha_ object_t add_infix_prule(string_t, object_t, double_t prec, int_t bind);
extern _rha_ object_t add_freefix_prule(string_t, object_t, tuple_t parts);

#define LEFT_BIND -1
#define NO_BIND    0
#define RIGHT_BIND 1

#endif
