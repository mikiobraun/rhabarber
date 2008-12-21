#include "int_fn.h"

#include <assert.h>
#include "object.h"
#include "messages.h"
#include "utils.h"

int int_plus(int x, int y) { return x+y; }
int int_minus(int x, int y) { return x-y; }
int int_times(int x, int y) { return x*y; }
int int_divide(int x, int y) { 
  if (y==0)
    rha_error("integer division by zero");
  else
    return x/y; 
  assert(false);
}
int int_neg(int x) { return -x; }
bool int_equalequal(int x, int y) { return x==y; }
bool int_notequal(int x, int y) { return x!=y; }
bool int_less(int x, int y) { return x<y; }
bool int_lessequal(int x, int y) { return x<=y; }
bool int_greater(int x, int y) { return x>y; }
bool int_greaterequal(int x, int y) { return x>=y; }

int int_cmp(int x, int y) {
  if (x==y) return 0;
  if (x<y) return -1;
  return 1;
}

int int_cast(any_t x)
{
  switch (ptype(x)) {
  case RHA_int:
    return UNWRAP_INT(x);
  case RHA_real_t:
    return (int) UNWRAP_REAL(x);
  default:
    rha_error("(casting) can't convert %o to 'int'", x);
    return 0;
  }
}

string_t int_to_string(int x)
{
  return sprint("%d", x);
}
