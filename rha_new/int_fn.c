#include "int_fn.h"

#include "object.h"
#include "messages.h"

int_t int_plus(int_t x, int_t y) { return x+y; }
int_t int_minus(int_t x, int_t y) { return x-y; }
int_t int_times(int_t x, int_t y) { return x*y; }
int_t int_divide(int_t x, int_t y) { return x/y; }
int_t int_neg(int_t x) { return -x; }
bool_t int_less(int_t x, int_t y) { return x<y; }

int_t int_cast(object_t x)
{
  switch (ptype(x)) {
  case INT_T:
    return UNWRAP_INT(x);
  case REAL_T:
    return (int_t) UNWRAP_REAL(x);
  default:
    rha_error("(casting) can't convert %o to 'int'", x);
    return 0;
  }
}
