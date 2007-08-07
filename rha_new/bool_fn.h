#ifndef BOOL_FN_H
#define BOOL_FN_H

#include <stdbool.h>
#include "rha_init.h"

extern _rha_ bool_t   bool_equal(bool_t, bool_t);
extern _rha_ bool_t   bool_not(bool_t);
// 'and' and 'or' are now with lazy macro-versions in core.c
extern _rha_ string_t bool_to_string(bool_t);

#endif
