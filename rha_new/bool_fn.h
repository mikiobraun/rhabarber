#ifndef BOOL_FN_H
#define BOOL_FN_H

#include <stdbool.h>
#include "rha_init.h"

extern _rha_ bool   bool_equalequal(bool, bool);
extern _rha_ string_t bool_to_string(bool);

extern _rha_ bool   bool_not(bool);
extern _rha_ bool   bool_or(bool, bool);
extern _rha_ bool   bool_and(bool, bool);

#endif
