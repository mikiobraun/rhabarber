#ifndef PARSE_H
#define PARSE_H

#include "rha_types.h"

extern _rha_ object_t parse(object_t root, string_t s);

extern       void     parse_init(object_t root, object_t module);


#define LEFT_BIND true
#define RIGHT_BIND false


#endif


