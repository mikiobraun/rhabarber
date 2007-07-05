/*
 * prule - the resolver for the grammar magic
 *
 * This file is part of rhabarber.
 *
 * (c) 2006 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */


#include "prule.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

//#define DEBUG
#include "debug.h"
#include "utils.h"
#include "messages.h"
#include "object.h"
#include "rhavt.h"

#include "tuple_tr.h"
#include "function_tr.h"
#include "builtin_tr.h"
#include "core.h"
#include "eval.h"


object_t resolve_prules(object_t env, object_t expr);

object_t recur_resolve_prules(object_t env, object_t expr)
{
  assert(HAS_TYPE(tuple, expr));
  for (int i=0; i<tuple_length(expr); i++)
    tuple_set(expr, i, resolve_prules(env, tuple_get(expr, i)));
  return expr;
}

object_t resolve_prules(object_t env, object_t expr)
{
  //fprint(stdout, "resolve_prules (before): %o\n", expr);

  if (!HAS_TYPE(tuple, expr))
    return expr;

  assert(HAS_TYPE(tuple, expr));  // what else?
  assert(tuple_length(expr) > 0); // correct?

  // so we know that expr is a non-empty tuple
  object_t t0 = tuple_get(expr, 0);
  if (HAS_TYPE(symbol, t0) 
      && ((symbol_equal_symbol(literal_sym, t0) 
	   || symbol_equal_symbol(do_sym, t0)))) {
    // [17, x, 42] or { x; y; z }
    return recur_resolve_prules(env, expr);
  }
  else if (HAS_TYPE(symbol, t0)
	   && (symbol_equal_symbol(tuplefy_sym, t0))) {
    // (tuplefy_sym, x)
    // this means that expr wasn't used as arguments, thus the
    // tuplefy_sym was just for grouping purposes
    if (tuple_length(expr)!=2)
      // (17, 42)  -> parse error
      rha_error("Arg list without function, parse error\n");
    return resolve_prules(env, tuple_get(expr, 1));
  }
  else {
    // it is a tuple containing a white-spaced list
    // without any particular heading symbol

    // (1) find the prule with the lowest precendence
    // loop over all entries and check the symbols whether they are prules
    double best_priority = 0.0;    // initially priority like functions
    object_t best_prule = 0;       // initially none
    object_t best_symbol = 0;      // the symbol that pointed to the highest ranked prule
    int tlen = tuple_length(expr);
    for (int i=0; i<tlen; i++) {
      object_t obj = tuple_get(expr, i);
      if (HAS_TYPE(symbol, obj)) {
	object_t f = rha_lookup(env, obj);
	// ??? do some check that we can call rha_priority() ???
	// to mikio: how to do it right?
	// ??? or was also rha_lookup() wrong ???
	if (f && (HAS_TYPE(function, f) || HAS_TYPE(builtin, f))) {
	  double priority =  rha_priority(f);
	  // did we find a prule with a higher priority?
	  if (priority > best_priority) {
	    // note that f is now for sure a prule, since priority > 0.0
	    best_priority = priority;
	    best_prule = f;
	    best_symbol = obj;
	  }
	}
      }
    }
    // did we find a prule?
    if (best_priority > 0.0) {
      // resolve prule by constructing a call to that prule
      object_t prule_call = tuple_new(3);
      tuple_set(prule_call, 0, best_prule);
      tuple_set(prule_call, 1, best_symbol);
      tuple_set(prule_call, 2, expr);     // the parse tree
      // run the prule itself to resolve it
      return recur_resolve_prules(env, rha_call(env, prule_call));
    }
    else {
      // otherwise resolve function calls
      object_t t0 = tuple_get(expr, 0);
      object_t fcc = resolve_prules(env, t0);  // fcc == function call collector
      for (int i=1; i<tlen; i++) {
	object_t ti = tuple_get(expr, i);
	if (!HAS_TYPE(tuple, ti))
	  rha_error("Function args must be in round brackets.\n");
	assert(tuple_length(ti) > 0);  // otherwise the grammar is wrong
	if (!symbol_equal_symbol(tuple_get(ti, 0), tuplefy_sym))
	  rha_error("Function args must be in round brackets.\n");
	// so now we now that ti = (tuplefy_sym, ...)
	// replace the tuplefy_sym with our call so far
	tuple_set(ti, 0, fcc);
	// resolve the args as well using prules
	for (int j=1; j<tuple_length(ti); j++)
	  tuple_set(ti, j, resolve_prules(env, tuple_get(ti, j)));
	fcc = ti;
      }
      return fcc;
    }
  }
  assert(1==0);  // never reach this point
}

