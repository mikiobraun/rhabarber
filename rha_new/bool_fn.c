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

bool bool_not(bool a) { return !a; }
bool bool_and(bool a, bool b) { return a && b; }
bool bool_or(bool a, bool b)  { return a || b; }

