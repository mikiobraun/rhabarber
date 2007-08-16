#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ object_t parse(object_t root, string_t s);
extern _rha_ object_t parse_file(object_t root, string_t fname);

extern _rha_ object_t resolve(object_t root, object_t expr);
extern       void     resolve_args(object_t env, tuple_t t);
extern       object_t resolve_list_by_prules(object_t env, list_t source);


extern       void     parse_init(object_t root, object_t module);


extern symbol_t semicolon_sym;
extern symbol_t comma_sym;
extern symbol_t dot_sym;
extern symbol_t tuple_forced_sym;
extern symbol_t resolving_sym;

//extern _rha_ object_t prule_failed_excp; // exception
extern object_t prule_failed_excp; // exception

#define LEFT_BIND true
#define RIGHT_BIND false


#endif


