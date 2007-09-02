#include "bool_fn.h"

#include <stdbool.h>
#include "alloc.h"
#include "utils.h"

bool bool_equalequal(bool b1, bool b2)
{
  return b1 == b2;
}


string_t bool_to_string(bool b)
{
  if (b) return sprint("true");
  else return sprint("false");
}

bool_t bool_not(bool_t a) { return !a; }
bool_t bool_and(bool_t a, bool_t b) { return a && b; }
bool_t bool_or(bool_t a, bool_t b)  { return a || b; }

