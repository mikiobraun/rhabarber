/*
 * rhavt - rhabarber virtual table
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef RHAVT_H
#define RHAVT_H

#include <stdbool.h>

/* These defines are necessary such that rha_stub can parse this file
   and we don't get a host of circular dependencies with object_t
   etc. */
#define object_t void *
struct tuple_s;
#define tuple_tr struct tuple_s *
struct symbol_s;
#define symbol_tr struct symbol_s *

/*
 * The virtual table
 */
struct rhavt
{
  object_t (*call)    (object_t, tuple_tr); 
  bool     (*callable)(object_t);
  void     (*bind)    (object_t, object_t);
  object_t (*calldef) (object_t, tuple_tr, object_t);
  object_t (*lookup)  (object_t, symbol_tr);
  object_t (*replace) (object_t, symbol_tr, object_t);
  double   (*priority)(object_t);
  bool     (*ismacro) (object_t);
};

extern struct rhavt default_vt;

/*
 * functions for calling elements in the virtual table
 */
extern object_t rha_call(object_t, tuple_tr); 
extern void     rha_bind(object_t, object_t);
extern bool     rha_callable(object_t);
extern object_t rha_calldef(object_t, tuple_tr, object_t);
extern object_t rha_lookup(object_t, symbol_tr);
extern object_t rha_replace(object_t, symbol_tr, object_t);
extern double   rha_priority(object_t);
extern bool     rha_ismacro(object_t);

extern bool     rha_bindable(object_t);
extern bool     rha_calldefable(object_t);

/*
 * some helper functions
 */

// call the function stored in call in t
//
// callcall copies t, replaces the t(0) by call,
// bind call to the previous t(0) and calls call.
// 
//  env: the calling environment
// call: the contents of t(0).rha_call
//    t: the original code tuple
extern object_t callcall(object_t env, object_t call, tuple_tr t);

// calls the function stored in call.calldef
//
// arguments are similar to callcall, o, l, r are those given to
// rha_calldef, and calldef is the contents of o.rha_calldef
extern object_t callcalldef(object_t o, object_t calldef, tuple_tr l, 
			    object_t r);

#undef object_t
#undef tuple_tr
#undef symbol_tr

#endif
