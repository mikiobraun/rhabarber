#include "bool_tr.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "object.h"
#include "primtype.h"
#include "string_tr.h"
#include "symbol_tr.h"

struct bool_s
{
  RHA_OBJECT;
  bool value;
};

#define CHECK(x) CHECK_TYPE(bool, x)

primtype_t bool_type = 0;

void bool_init(void)
{
  if (!bool_type) {
    bool_type = primtype_new("bool", sizeof(struct bool_s));
    (void) bool_new(false);
  }
}


bool_tr bool_new(bool value)
{
  bool_tr i = object_new(bool_type);
  i->value = value;
  return i;
}


bool_tr bool_copy(bool_tr b)
{
  return bool_new(bool_get(b));
}


bool bool_get(bool_tr b)
{
  CHECK(b);
  return b->value;
}


bool bool_equal_bool(bool b1, bool b2)
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

bool bool_from_none(none_tr n)
{
  return false;
}


#undef CHECK
