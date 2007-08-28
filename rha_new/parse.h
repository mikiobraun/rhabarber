#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ object_t parse(object_t root, string_t s);
extern _rha_ object_t parse_file(object_t root, string_t fname);



extern _rha_ object_t resolve(object_t root, list_t source);
extern       object_t resolve_patterns(object_t env, list_t source);


extern       void     parse_init(object_t root, object_t module);


// note that the following symbols can NOT go to 'rha_config.d' since
// the string for 'dot_sym' is ':' and not 'dot'
extern symbol_t dot_sym;

//extern _rha_ object_t prule_failed_excp; // exception
extern object_t prule_failed_excp; // exception

#define LEFT_BIND true
#define RIGHT_BIND false


#endif


