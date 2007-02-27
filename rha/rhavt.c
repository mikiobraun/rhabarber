#include "rhavt.h"

//#define DEBUG
#include "debug.h"
#include "primtype.h"
#include "object.h"
#include "messages.h"
#include "core.h"
#include "none_tr.h"

#include "tuple_tr.h"
#include "eval.h"

/************************************************************
 * Invoking
 */

object_t rha_call(object_t env, tuple_tr t)
{
  assert(tuple_length(t)>0);
  object_t o = tuple_get(t, 0);
  printdebug("rha_call called for object %o\n", o);
  return primtype_vt(object_primtype(o))->call(env, t);
}

static
object_t default_call(object_t env, tuple_tr t)
{
  assert(tuple_length(t)>0);
  object_t o = tuple_get(t, 0);

  // Let's see if the target implements rha_call
  object_t call = object_lookup(o, call_sym);
  if(call) {
    // If yes, replace the target by this function, bind it to the
    // target and call the method.
    return callcall(env, call, t);
  }
  
  rha_error("Cannot call object %o!\n", o);
  return 0; // cannot happen
}

// call the call object
object_t callcall(object_t env, object_t call, tuple_tr t)
{
  tuple_tr ct = tuple_copy(t);
  tuple_set(ct, 0, call);
  rha_bind(call, tuple_get(t, 0));
  return rha_call(env, ct);
}

/************************************************************
 * check for callable
 */

bool rha_callable(object_t obj)
{
  return primtype_vt(object_primtype(obj))->callable(obj);
}

static
bool default_callable(object_t obj)
{
  return primtype_vt(object_primtype(obj))->call != default_call
    || object_lookup(obj, call_sym);
}

/************************************************************
 * for expressions of the type obj(arg1, ..., arg2) = 0
 */

object_t rha_calldef(object_t o, tuple_tr l, object_t r)
{
  return primtype_vt(object_primtype(o))->calldef(o, l, r);
}

static
object_t default_calldef(object_t o, tuple_tr l, object_t r)
{
  object_t calldef = object_lookup(o, calldef_sym);
  if(calldef) {
    // if method has callassing slot, construct the call, bind the
    // function to "o" and call the method.
    callcalldef(o, calldef, l, r);
  }
  else
    rha_error("Cannot call calldef object %o.\n", o);
  return 0;
}

bool rha_calldefable(object_t obj)
{
  return primtype_vt(object_primtype(obj))->calldef != default_calldef
    || object_lookup(obj, calldef_sym);
}

object_t callcalldef(object_t o, object_t calldef, tuple_tr l, object_t r)
{
  tuple_tr ct = tuple_make3(calldef, l, r);
  rha_bind(calldef, o);
  return rha_call(none_obj, ct);
}

/************************************************************
 * bind function object to object for next call
 */

void rha_bind(object_t obj, object_t this)
{
  primtype_vt(object_primtype(obj))->bind(obj, this);
}


static
void default_bind(object_t obj, object_t this)
{
  return;
}

bool rha_bindable(object_t obj)
{
  return primtype_vt(object_primtype(obj))->bind != default_bind;
}

/************************************************************
 * lookup up a symbol
 */

object_t rha_lookup(object_t obj, symbol_tr s)
{
  CHECK_TYPE(symbol, s);
  return primtype_vt(object_primtype(obj))->lookup(obj, s);
}

static
object_t default_lookup(object_t obj, symbol_tr s)
{
  return object_lookup(obj, s);
}

/************************************************************
 * replace up a symbol
 */

object_t rha_replace(object_t obj, symbol_tr s, object_t nv)
{
  return primtype_vt(object_primtype(obj))->replace(obj, s, nv);
}

static
object_t default_replace(object_t obj, symbol_tr s, object_t nv)
{
  return object_replace(obj, s, nv);
}

/************************************************************
 * get the priority for a function/builtin/overloaded
 */

double rha_priority(object_t obj)
{
  return primtype_vt(object_primtype(obj))->priority(obj);
}

static
double default_priority(object_t obj)
{
  return 0.0;  // no particular priority
}

/************************************************************
 * get the 'ismacro'-property for a function/builtin/overloaded
 */

bool rha_ismacro(object_t obj)
{
  return primtype_vt(object_primtype(obj))->ismacro(obj);
}

static
bool default_ismacro(object_t obj)
{
  return false;  // in dubio: not a macro
}

/************************************************************
 * the default virtual table
 */

struct rhavt default_vt =  { 
  .call = default_call,
  .callable = default_callable,
  .bind = default_bind,
  .calldef = default_calldef,
  .lookup = default_lookup,
  .replace = default_replace,
  .priority = default_priority,
  .ismacro = default_ismacro
};

