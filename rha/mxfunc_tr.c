/*
 * mxfunc - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#include "mxfunc_tr.h"

#include "object.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "eval.h"
#include "matlab_tr.h"
#include "none_tr.h"
#include "int_tr.h"

struct mxfunc_s
{
  RHA_OBJECT;
  string_t name;
};

#define CHECK(x) CHECK_TYPE(mxfunc, x)

primtype_t mxfunc_type = 0;
static struct rhavt mxfunc_vt;

static object_t mxfunc_vt_call(object_t env, tuple_tr t);


void mxfunc_init(void)
{
  if (!mxfunc_type) {
    mxfunc_type = primtype_new("mxfunc", sizeof(struct mxfunc_s));
    mxfunc_vt = default_vt;
    mxfunc_vt.call = mxfunc_vt_call;
    primtype_setvt(mxfunc_type, &mxfunc_vt);
    (void) mxfunc_new("");
  }
}


mxfunc_tr mxfunc_new(string_t name)
{
  mxfunc_tr newmxfunc = object_new(mxfunc_type);
  newmxfunc->name = gc_strdup(name);

  object_assign(newmxfunc, symbol_new("domain"), matlab_domain); 

  return newmxfunc;
}


static
object_t mxfunc_vt_call(object_t env, tuple_tr t)
{
  mxfunc_tr f = tuple_get(t, 0);
  CHECK(f);
  
  int mxin, mxout;
  if(tuple_length(t) == 1) {
    mxout = 1;
    mxin = 0;
  }
  else {
    if(!HAS_TYPE(int, tuple_get(t, 1)))
      rha_error("when calling a matlab function, the first argument " 
		"specifies the number of output arguments.");

    mxout = int_get(tuple_get(t, 1));
    mxin = tuple_length(t) - 2;
  }
  
  if(!mxin) {
    return matlab_call(f->name, mxout, none_obj);
  }
  else {
    t = eval_fctcall_hook(env, t);

    tuple_tr args = tuple_new(mxin);
    for(int i = 0; i < mxin; i++)
      tuple_set(args, i, tuple_get(t, i+2));
    return matlab_call(f->name, mxout, args);
  }

  return none_obj;
}


#undef CHECK
