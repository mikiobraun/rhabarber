#include "builtin_tr.h"

#include <assert.h>
#include <stdio.h>

#include "eval.h"

#include "object.h"
#include "primtype.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "none_tr.h"
#include "utils.h"

#include "tuple_tr.h"

struct builtin_s
{
  RHA_OBJECT;
  builtinfct_t *fn;
  object_t *this;
  double priority;  // used for prules, large priority iff low precendence
  bool ismacro;     // flag that decides whether args should be evaluated
};

#define CHECK(x) CHECK_TYPE(builtin, x)

primtype_t builtin_type = 0;
struct rhavt builtin_vt;

static object_t builtin_vt_call(object_t env, tuple_tr in);
static void builtin_vt_bind(object_t env, object_t obj);
static double builtin_vt_priority(object_t obj);
static bool builtin_vt_ismacro(object_t obj);

void builtin_init(void)
{  
  if (!builtin_type) {
    builtin_type = primtype_new("builtin", sizeof(struct builtin_s));
    builtin_vt = default_vt;
    builtin_vt.call = builtin_vt_call;
    builtin_vt.bind = builtin_vt_bind;
    builtin_vt.priority = builtin_vt_priority;
    builtin_vt.ismacro = builtin_vt_ismacro;
    primtype_setvt(builtin_type, &builtin_vt);

    (void) builtin_new(NULL);
  }
}


builtin_tr builtin_new(builtinfct_t fn)
{
  builtin_tr b = object_new(builtin_type);
  b->fn = fn;
  b->this = 0;
  b->priority = 0.0;
  b->ismacro = false;
  return b;
}


builtin_tr builtin_new_prule(builtinfct_t fn, double priority)
{
  builtin_tr b = builtin_new(fn);
  b->priority = priority;
  return b;
}


builtin_tr builtin_new_macro(builtinfct_t fn)
{
  builtin_tr b = builtin_new(fn);
  b->ismacro = true;
  return b;
}


object_t builtin_vt_call(object_t env, tuple_tr t)
{
  assert(HAS_TYPE(tuple, t));
  assert(tuple_length(t)>0);
  builtin_tr b = tuple_get(t, 0);
  assert(HAS_TYPE(builtin, b));

  // save this pointer first
  object_t this = b->this;
  b->this = 0;

  // call fctcall_hook
  t = eval_fctcall_hook(env, t);

  // call builtin-fct
  object_t retval = (*b->fn)(this, env, t);
  return retval ? retval : none_obj;
}


void builtin_vt_bind(object_t obj, object_t this)
{
  CHECK(obj);
  ((builtin_tr)obj)->this = this;
}


double builtin_vt_priority(object_t obj)
{
  CHECK(obj);
  return ((builtin_tr)obj)->priority;
}


bool builtin_vt_ismacro(object_t obj)
{
  CHECK(obj);
  return ((builtin_tr)obj)->ismacro;
}
