#include "exception_tr.h"

#include <stdio.h>
#include <assert.h>

#include "object.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"
#include "utils.h"


struct exception_s
{
  RHA_OBJECT;
  string_t msg;
};

#define CHECK(x) CHECK_TYPE(exception, x)

primtype_t exception_type = 0;

void exception_init(void)
{
  if (!exception_type) {
    exception_type = primtype_new("exception", sizeof(struct exception_s));
    (void) exception_new("");
  }
}


exception_tr exception_new(string_t msg)
{
  exception_tr e = object_new(exception_type);
  e->msg = gc_strdup(msg);
  return e;
}


string_t exception_to_string(exception_tr e)
{
  CHECK(e);
  return e->msg;
}


#undef CHECK
