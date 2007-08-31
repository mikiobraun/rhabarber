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

bool bool_not(bool b) 
{
  return !b;
}


// lazy 'and'
bool_t bool_and(bool_t a, bool_t b)
{
  if (!a) return false;
  if (!b) return false;
  return true;
}

// lazy 'or'
bool_t bool_or(bool_t a, bool_t b)
{
  if (a) return true;
  if (b) return true;
  return false;
}

