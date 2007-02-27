/*
 * prule - the resolver
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
#include "core.h"
#include "eval.h"

object_t resolve_prules(object_t env, object_t expr)
{
  fprint(stdout, "resolve_prules before: %o\n", expr);

  if (!HAS_TYPE(tuple, expr) || (tuple_length(expr) == 0)) return expr;
  // so we know at this point that expr is a non-empty tuple
  object_t t0 = tuple_get(expr, 0);
  if (!HAS_TYPE(symbol, t0)                      // no symbol => no group/tuplefy
      || !(symbol_equal_symbol(group_sym, t0)         // ignore group-tuples
	   || symbol_equal_symbol(tuplefy_sym, t0)))  // ignore tuplefy-tuples
    {
      // (1) find the prule with the lowest precendence
      // loop over all entries and check the symbols whether they are prules
      double best_priority = 0.0;    // initially priority like functions
      object_t best_prule = 0;  // initially none
      int tlen = tuple_length(expr);
      for (int i=0; i<tlen; i++) {
	object_t obj = tuple_get(expr, i);
	if (HAS_TYPE(symbol, obj)) {
	  object_t f = rha_lookup(env, obj);
	  if (f) {
	    double priority =  rha_priority(f);
	    //fprint(stdout, "priority, %f\n", priority);
	    // did we find a prule with a higher priority?
	    if (priority > best_priority) {
	      // note that f is now for sure a prule, since priority > 0.0
	      best_priority = priority;
	      best_prule = f;
	    }
	  }
	}
      }
      // (2) resolve prule or function calls
      // do we have to resolve a prule?
      if (best_priority > 0.0) {
	// yes, so call it
	object_t acall = tuple_new(2);
	tuple_set(acall, 0, best_prule);
	tuple_set(acall, 1, expr);
	fprint(stdout, "resolve_prules acall: %o\n", acall);
	// run the prule itself to resolve it
	expr = rha_call(env, acall);
	// now the first entry of expr be a function and the other entries its args
      }
      else {
	// no, then resolve function calls instead
	string_t msg = "Can't resolve prules/function calls.";
	object_t t0 = tuple_get(expr, 0);
	if (iscallof(group_sym, t0)) {
	  assert(tuple_length(t0)==2);
	  t0 = tuple_get(t0, 1);
	}
	object_t cur = resolve_prules(env, t0);
	for (int i=1; i<tlen; i++) {
	  object_t ti = tuple_get(expr, i);
	  if (iscallof(group_sym, ti) || iscallof(tuplefy_sym, ti)) {
	    ti = resolve_prules(env, ti);
	    tuple_set(ti, 0, cur);
	    cur = ti;
	  }
	  else rha_error(msg);
	}
        fprint(stdout, "resolve_prules (function calls): %o\n", cur);
	return cur; // we are already done, since we did the recursive calls ourselves
      }
    }

  // finally call resolve_prule on each entry (but the zeroth, where
  // the function to be called is located)
  int tlen = tuple_length(expr);   // the length might have changed
  for (int i=1; i<tlen; i++) {
    tuple_set(expr, i, resolve_prules(env, tuple_get(expr, i)));
  }
  fprint(stdout, "resolve_prules (final): %o\n", expr);
  return expr;
}
