#include "bool_fn.h"

#include <stdbool.h>
#include "alloc.h"

bool bool_equal(bool b1, bool b2)
{
  return b1 == b2;
}


string_t bool_to_string(bool b)
{
  if (b) return gc_strdup("true");
  else return gc_strdup("false");
}

bool not_fn(bool b)
{
  return !b;
}


// lazy 'and'
bool_t and_fn(bool_t a, bool_t b)
{
  if (!a) return false;
  if (!b) return false;
  return true;
}

// lazy 'or'
bool_t or_fn(bool_t a, bool_t b)
{
  if (a) return true;
  if (b) return true;
  return false;
}

