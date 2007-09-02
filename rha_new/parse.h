#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ any_t parse(any_t root, string_t s);
extern _rha_ any_t parse_file(any_t root, string_t fname);



extern _rha_ any_t resolve(any_t root, list_t source);
extern       any_t resolve_patterns(any_t env, list_t source);


extern       void     parse_init(any_t root, any_t module);


// note that the following symbols can NOT go to 'rha_config.d' since
// the string for 'dot_sym' is ':' and not 'dot'
extern symbol_t dot_sym;

//extern _rha_ any_t prule_failed_excp; // exception
extern any_t prule_failed_excp; // exception

#define LEFT_BIND true
#define RIGHT_BIND false


#endif


