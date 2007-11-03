#ifndef REAL_FN_H
#define REAL_FN_H

#include "rha_types.h"

extern _rha_ real_t  real_plus(real_t x, real_t y);
extern _rha_ real_t  real_minus(real_t x, real_t y);
extern _rha_ real_t  real_times(real_t x, real_t y);
extern _rha_ real_t  real_divide(real_t x, real_t y);
extern _rha_ real_t  real_neg(real_t x);
extern _rha_ bool real_equalequal(real_t x, real_t y);
extern _rha_ bool real_notequal(real_t x, real_t y);
extern _rha_ bool real_less(real_t x, real_t y);
extern _rha_ bool real_lessequal(real_t x, real_t y);
extern _rha_ bool real_greater(real_t x, real_t y);
extern _rha_ bool real_greaterequal(real_t x, real_t y);

extern _rha_ real_t  real_cmp(real_t x, real_t y);
extern _rha_ real_t  real_cast(any_t x);

extern _rha_ real_t  real_exp(real_t x);
extern _rha_ real_t  real_sin(real_t x);
extern _rha_ real_t  real_cos(real_t x);

extern _rha_ string_t real_to_string(real_t x);
#endif
