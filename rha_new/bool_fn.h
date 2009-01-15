#ifndef BOOL_FN_H
#define BOOL_FN_H

#include <stdbool.h>
#include "rha_init.h"

extern  bool   bool_equalequal(bool, bool);
extern  string_t bool_to_string(bool);

extern  bool   bool_not(bool);
extern  bool   bool_or(bool, bool);
extern  bool   bool_and(bool, bool);

#endif
