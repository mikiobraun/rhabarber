#include "int_fn.h"

#include "object.h"
#include "messages.h"
#include "utils.h"

int_t int_plus(int_t x, int_t y) { return x+y; }
int_t int_minus(int_t x, int_t y) { return x-y; }
int_t int_times(int_t x, int_t y) { return x*y; }
int_t int_divide(int_t x, int_t y) { 
  if (y==0)
    rha_error("integer division by zero");
  else
    return x/y; 
}
int_t int_neg(int_t x) { return -x; }
bool_t int_equalequal(int_t x, int_t y) { return x==y; }
bool_t int_notequal(int_t x, int_t y) { return x!=y; }
bool_t int_less(int_t x, int_t y) { return x<y; }
bool_t int_lessequal(int_t x, int_t y) { return x<=y; }
bool_t int_greater(int_t x, int_t y) { return x>y; }
bool_t int_greaterequal(int_t x, int_t y) { return x>=y; }

int_t int_cmp(int_t x, int_t y) {
  if (x==y) return 0;
  if (x<y) return -1;
  return 1;
}

int_t int_cast(any_t x)
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

string_t int_to_string(int_t x)
{
  return sprint("%d", x);
}
