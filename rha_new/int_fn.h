#ifndef INT_FN_H
#define INT_FN_H

#include "rha_types.h"

extern _rha_ int  int_plus(int x, int y);
extern _rha_ int  int_minus(int x, int y);
extern _rha_ int  int_times(int x, int y);
extern _rha_ int  int_divide(int x, int y);
extern _rha_ int  int_neg(int x);
extern _rha_ bool int_equalequal(int x, int y);
extern _rha_ bool int_notequal(int x, int y);
extern _rha_ bool int_less(int x, int y);
extern _rha_ bool int_lessequal(int x, int y);
extern _rha_ bool int_greater(int x, int y);
extern _rha_ bool int_greaterequal(int x, int y);

extern _rha_ int  int_cmp(int x, int y);
extern _rha_ int  int_cast(any_t x);

extern _rha_ string_t int_to_string(int x);
#endif
