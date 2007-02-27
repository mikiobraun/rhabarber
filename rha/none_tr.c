#include "none_tr.h"

#include <stdio.h>
#include <assert.h>

#include "object.h"
#include "primtype.h"
#include "string_tr.h"
#include "symbol_tr.h"

struct none_s
{
  RHA_OBJECT;
  /* empty */
};

#define CHECK(x) CHECK_TYPE(none, x)

primtype_t none_type = 0;
none_tr none_obj = 0;

void none_init(void)
{
  if (!none_type) {
    none_type = primtype_new("none", sizeof(struct none_s));
    none_obj = none_new();
  }
}

none_tr none_new()
{
  if(!none_obj)
    none_obj = object_new(none_type);
  return none_obj;
}

bool none_is_none(none_tr n)
{
  return object_primtype(n) == none_type;
}

string_t none_to_string(none_tr n)
{
  return gc_strdup("none");
}

bool none_equal_none(none_tr n1, none_tr n2)
{
  return true;
}


#undef CHECK
