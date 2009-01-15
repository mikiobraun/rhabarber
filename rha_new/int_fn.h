#ifndef INT_FN_H
#define INT_FN_H

#include "rha_types.h"

extern  int  int_plus(int x, int y);
extern  int  int_minus(int x, int y);
extern  int  int_times(int x, int y);
extern  int  int_divide(int x, int y);
extern  int  int_neg(int x);
extern  bool int_equalequal(int x, int y);
extern  bool int_notequal(int x, int y);
extern  bool int_less(int x, int y);
extern  bool int_lessequal(int x, int y);
extern  bool int_greater(int x, int y);
extern  bool int_greaterequal(int x, int y);

extern  int  int_cmp(int x, int y);
extern  int  int_cast(any_t x);

extern  string_t int_to_string(int x);
#endif
