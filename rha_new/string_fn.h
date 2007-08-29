#ifndef STRING_FN_H
#define STRING_FN_H

#include "rha_types.h"

extern _rha_ string_t string_copy(string_t other);
extern _rha_ string_t string_concat(string_t s1, string_t s2);
extern _rha_ int_t    string_len(string_t s);
extern _rha_ bool_t   string_equalequal(string_t s1, string_t s2);
#endif
