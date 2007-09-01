#include "real_fn.h"

#include <math.h>
#include "object.h"
#include "messages.h"
#include "utils.h"

real_t real_plus(real_t x, real_t y) { return x+y; }
real_t real_minus(real_t x, real_t y) { return x-y; }
real_t real_times(real_t x, real_t y) { return x*y; }
real_t real_divide(real_t x, real_t y) { return x/y; }
real_t real_neg(real_t x) { return -x; }
bool_t real_equalequal(real_t x, real_t y) { return x==y; }
bool_t real_notequal(real_t x, real_t y) { return x!=y; }
bool_t real_less(real_t x, real_t y) { return x<y; }
bool_t real_lessequal(real_t x, real_t y) { return x<=y; }
bool_t real_greater(real_t x, real_t y) { return x>y; }
bool_t real_greaterequal(real_t x, real_t y) { return x>=y; }
real_t real_exp(real_t x) { return exp(x); }
real_t real_cos(real_t x) { return cos(x); }
real_t real_sin(real_t x) { return sin(x); }

real_t real_cmp(real_t x, real_t y) {
  if (x==y) return 0;
  if (x<y) return -1;
  return 1;
}

real_t real_cast(object_t x)
{
  switch (ptype(x)) {
  case REAL_T:
    return UNWRAP_REAL(x);
  case INT_T:
    return (real_t) UNWRAP_INT(x);
  default:
    rha_error("(casting) can't convert %o to 'real'", x);
    return 0;
  }
}

string_t real_to_string(real_t x)
{
  return sprint("%f", x);
}
