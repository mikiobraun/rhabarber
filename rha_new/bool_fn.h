#ifndef BOOL_FN_H
#define BOOL_FN_H

#include <stdbool.h>
#include "rha_init.h"

extern _rha_ bool_t   bool_equal(bool_t, bool_t);
extern _rha_ string_t bool_to_string(bool_t);

extern _rha_ bool_t   not_fn(bool_t);
extern _rha_ bool_t   or_fn(bool_t a, bool_t b);
extern _rha_ bool_t   and_fn(bool_t a, bool_t b);

#endif
