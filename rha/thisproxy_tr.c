/*
 * thisproxy - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#include "thisproxy_tr.h"

#include "object.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "rhavt.h"
#include "method_tr.h"
#include "core.h"
#include "utils.h"

struct thisproxy_s
{
  RHA_OBJECT;
  object_t this;
};

#define CHECK(x) CHECK_TYPE(thisproxy, x)

primtype_t thisproxy_type = 0;
struct rhavt thisproxy_vt;

static object_t thisproxy_vt_lookup(object_t o, symbol_tr s);
static object_t thisproxy_vt_replace(object_t o, symbol_tr s, object_t v);

void thisproxy_init(void)
{
  if (!thisproxy_type) {
    thisproxy_type = primtype_new("thisproxy", sizeof(struct thisproxy_s));
    thisproxy_vt = default_vt;
    thisproxy_vt.lookup = thisproxy_vt_lookup;
    thisproxy_vt.replace = thisproxy_vt_replace;
    primtype_setvt(thisproxy_type, &thisproxy_vt);
    (void) thisproxy_new(0);
  }
}

thisproxy_tr thisproxy_new(object_t this)
{
  thisproxy_tr tp = object_new(thisproxy_type);
  tp->this = this;
  return tp;
}

string_t thisproxy_to_string(thisproxy_tr tp)
{
  CHECK(tp);
  return sprint("[thisproxy bound to %o]", tp->this);
}

/**********************************************************************
 *
 * Virtual table functions
 *
 **********************************************************************/

object_t thisproxy_vt_lookup(object_t env, symbol_tr s)
{
  CHECK(env);
  CHECK_TYPE(symbol, s);

  thisproxy_tr tp = env;
  // first: resolve "this"
  if (symbol_equal_symbol(s, this_sym))
    return tp->this;
  // next lookup in myself
  object_t o = object_lookuplocal(env, s);
  // if not, delegate to "this"
  if(!o) {
    o = rha_lookup(tp->this, s);
    if(o && rha_bindable(o))
      o = method_new(tp->this, o);
  }
  // finally, delegate to parents
  if(!o && object_parent(env)) {
    o = rha_lookup(object_parent(env), s);
  }
  return o;
}

object_t thisproxy_vt_replace(object_t env, symbol_tr s, object_t v)
{
  CHECK(env);
  thisproxy_tr tp = env;
  object_t o = rha_replace(tp->this, s, v);
  if(!o)
    return rha_replace(object_parent(tp), s, v);
  return o;
}

#undef CHECK
