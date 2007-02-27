#include "method_tr.h"

#include <assert.h>
#include <stdio.h>
#include "messages.h"
#include "eval.h"
#include "utils.h"
#include "core.h"
#include "object.h"
#include "primtype.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "function_tr.h"
#include "builtin_tr.h"
#include "overloaded_tr.h"
#include "plain_tr.h"

struct method_s
{
  RHA_OBJECT;
  object_t recv; // the call's receiver
  object_t fn;   // the function itself
};

#define CHECK(f) CHECK_TYPE(method, f)

primtype_t method_type = 0;
struct rhavt method_vt;

static object_t    method_vt_call(object_t env, tuple_tr in);
static bool        method_vt_ismacro(object_t f);

void method_init()
{
  if (!method_type) {
    method_type = primtype_new("method", sizeof(struct method_s));
    method_vt = default_vt;
    method_vt.call = &method_vt_call;
    method_vt.ismacro = &method_vt_ismacro;
    primtype_setvt(method_type, &method_vt);

    (void) method_new(0, 0);
  }
}


method_tr method_new(object_t this, object_t fn)
{
  method_tr f = object_new(method_type);
  f->recv = this;
  f->fn = fn;

  return f;
}


string_t method_to_string(method_tr f)
{
  CHECK(f);
  return sprint("[method %o bound to %p]", f->fn, f->recv);
}


/**********************************************************************
 *
 * virtual functions
 *
 **********************************************************************/

static
object_t method_vt_call(object_t env, tuple_tr t)
{
  assert(HAS_TYPE(tuple, t));
  assert(tuple_length(t)>0);
  method_tr f = tuple_get(t, 0);
  assert(HAS_TYPE(method, f));

  object_t this = f->recv;
  object_t fn = f->fn;

  // bind the function to the receiver
  rha_bind(fn, this);
  
  // replace method with the function
  tuple_set(t, 0, fn);

  // call adjusted method
  return rha_call(env, t);
}


static
bool method_vt_ismacro(object_t f)
{
  CHECK(f);

  // pass the ismacro-request on
  return rha_ismacro(((method_tr) f)->fn);
}
