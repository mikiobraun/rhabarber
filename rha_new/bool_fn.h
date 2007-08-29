#ifndef BOOL_FN_H
#define BOOL_FN_H

#include <stdbool.h>
#include "rha_init.h"

extern _rha_ bool_t   bool_equalequal(bool_t, bool_t);
extern _rha_ string_t bool_to_string(bool_t);

extern _rha_ bool_t   bool_not(bool_t);
extern _rha_ bool_t   bool_or(bool_t a, bool_t b);
extern _rha_ bool_t   bool_and(bool_t a, bool_t b);

#endif
