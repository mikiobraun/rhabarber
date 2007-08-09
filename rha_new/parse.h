#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ object_t parse(object_t root, string_t s);

extern _rha_ object_t keyword(string_t, int_t);

extern _rha_ object_t macro_prule(string_t, object_t);
extern _rha_ object_t postfix_prule(string_t, object_t);
extern _rha_ object_t prefix_prule(string_t, object_t);
extern _rha_ object_t infix_prule(string_t, object_t, double_t prec, int_t bind);
extern _rha_ object_t freefix_prule(string_t, object_t, tuple_t parts);

#define LEFT_BIND -1
#define NO_BIND    0
#define RIGHT_BIND 1

#endif


//object_t prules = new();
//assign(root, symbol_new("prules"), prules);
//assign(prules, symbol_new("++"), prefix_prule("++", f));
