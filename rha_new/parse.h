#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ object_t parse(object_t root, string_t s);

extern object_t parse_init(object_t root);

extern _rha_ object_t keyword(string_t, int_t);

extern _rha_ object_t macrofix_prule(string_t, object_t);
extern _rha_ object_t postfix_prule(string_t, object_t);
extern _rha_ object_t prefix_prule(string_t, object_t);
extern _rha_ object_t infix_prule(string_t, object_t, real_t prec, int_t bind);
extern _rha_ object_t freefix_prule(string_t, object_t, tuple_t parts);

#define LEFT_BIND -1
#define NO_BIND    0
#define RIGHT_BIND 1

#define MACROFIX 0
#define POSTFIX  1
#define PREFIX   2
#define INFIX    3
#define FREEFIX  4

#define MACROPREC   1.0
#define POSTPREC    2.0
#define PREPREC     3.0
// INFIX should be between 3.0 and 5.0 for numerical infix operators
#define FREEPREC    5.0
// INFIX should be larger than 5.0 for assignments
// INFIX for comma, semicolon even larger


#endif


//object_t prules = new();
//assign(root, symbol_new("prules"), prules);
//assign(prules, symbol_new("++"), prefix_prule("++", f));
