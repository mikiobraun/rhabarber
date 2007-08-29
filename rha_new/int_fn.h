#ifndef INT_FN_H
#define INT_FN_H

#include "rha_types.h"

extern _rha_ int_t  int_plus(int_t x, int_t y);
extern _rha_ int_t  int_minus(int_t x, int_t y);
extern _rha_ int_t  int_times(int_t x, int_t y);
extern _rha_ int_t  int_divide(int_t x, int_t y);
extern _rha_ int_t  int_neg(int_t x);
extern _rha_ bool_t int_equalequal(int_t x, int_t y);
extern _rha_ bool_t int_notequal(int_t x, int_t y);
extern _rha_ bool_t int_less(int_t x, int_t y);
extern _rha_ bool_t int_lessequal(int_t x, int_t y);
extern _rha_ bool_t int_greater(int_t x, int_t y);
extern _rha_ bool_t int_greaterequal(int_t x, int_t y);

extern _rha_ int_t  int_cmp(int_t x, int_t y);
extern _rha_ int_t  int_cast(object_t x);
#endif
