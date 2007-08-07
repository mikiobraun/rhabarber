#include "bool_fn.h"

#include <stdbool.h>
#include "alloc.h"

bool bool_equal(bool b1, bool b2)
{
  return b1 == b2;
}


bool bool_not(bool b)
{
  return !b;
}

bool bool_and_bool(bool b1, bool b2)
{
  return b1 && b2;
}

bool bool_or_bool(bool b1, bool b2)
{
  return b1 || b2;
}

string_t bool_to_string(bool b)
{
  if (b) return gc_strdup("true");
  else return gc_strdup("false");
}
