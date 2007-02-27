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
#include "function_tr.h"
#include "builtin_tr.h"
#include "core.h"
#include "eval.h"

// forward declarations
object_t resolve_function_calls_and_dots(object_t env, object_t expr);

object_t resolve_prules(object_t env, object_t expr)
{
  //fprint(stdout, "resolve_prules (before): %o\n", expr);

  if (HAS_TYPE(symbol, expr)) {
    // some prules do not come enclosed in tuples, 
    // examples: break, deliver, return called without args
    // anything else?  note that we still need b_break, b_deliver, b_return
    // in core.c which deal with arguments as well
    if (symbol_equal_symbol(expr, break_sym))
      expr = tuple_make(1, op_break_sym);
    else if (symbol_equal_symbol(expr, deliver_sym))
      expr = tuple_make(1, op_deliver_sym);
    else if (symbol_equal_symbol(expr, return_sym))
      expr = tuple_make(1, op_return_sym);
    return expr;
  }

  if (!HAS_TYPE(tuple, expr) || (tuple_length(expr) == 0)) {
    // this case stops the recursive calls of resolve_prules (at the end)
    //fprint(stdout, "resolve_prules (after): %o\n", expr);
    return expr;
  }

  // so we know at this point that expr is a non-empty tuple
  object_t t0 = tuple_get(expr, 0);
  object_t result = 0;
  if (HAS_TYPE(symbol, t0) && symbol_equal_symbol(group_sym, t0)) {
    // (group_sym x)
    assert(tuple_length(expr) == 2);
    // note that for the group_sym, we additionally have to call resolve_prules ourselves
    // because otherwise below it wouldn't be called on an extracted tuple, but only its elements
    //   expr = (group, (fn, (tuplefy), 17))
    // becomes by tuple_get(expr, 1)
    //   (fn (tuplefy) 17)
    // here it is not enough to call resolve_prules on each argument,
    // we also have to call it on the whole tuple
    result = resolve_prules(env, tuple_get(expr, 1));
    return result;
  }
  else if (HAS_TYPE(symbol, t0) && symbol_equal_symbol(tuplefy_sym, t0)) {
    // (tuplefy_sym, x, y)
    // do nothing (calling resolve_prules each argument is done below)
    result = expr;
  }
  else {
    // it is a tuple, but neither with group_sym nor with tuplefy_sym

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
      tuple_set(prule_call, 2, expr);        // the parse tree
      result = rha_call(env, prule_call); // run the prule itself to resolve it
    }
    else {
      // resolve function calls and dots
      result = resolve_function_calls_and_dots(env, expr);
      // since resolve_function_calls_and_dots calls resolve_prules we are done
      return result;
    }
  }
  
  // finally call resolve_prule on each entry if 'expr' is a tuple
  if (HAS_TYPE(tuple, result)) {
    int tlen = tuple_length(result);   // the length of result has probably been changed
    for (int i=0; i<tlen; i++) {
      tuple_set(result, i, resolve_prules(env, tuple_get(result, i)));
    }
  }
  //fprint(stdout, "resolve_prules (final): %o\n", result);
  return result;
}

object_t resolve_function_calls_and_dots(object_t env, object_t expr)
     // function calls and dots/nobinddots must be dealt with simultaneously
{
  //fprint(stdout, "resolve_function_calls_and_dots (before): %o\n", expr);
  assert(HAS_TYPE(tuple, expr));
  object_t fcc = 0;  // fcc == function call collector, 
                     // note that fcc might contain a single symbol
  bool with_dot = false;  // a flag: did we see a dot?
  object_t dot_expr = 0;  // this helps us constructing dot expressions
  int tlen = tuple_length(expr);
  for (int i=0; i<tlen; i++) {
    object_t ti = tuple_get(expr, i);
    if (HAS_TYPE(symbol, ti) &&
	(symbol_equal_symbol(ti, dot_sym) || 
	 symbol_equal_symbol(ti, nobinddot_sym))) {
      // do we first have to finish an old dot?
      if (with_dot) {
	if (!fcc)
	  rha_error("dots must have rhs.\n");
	tuple_set(dot_expr, 2, fcc);
	fcc = dot_expr;
      }
      // start a new dot_expr
      if (fcc) {
	dot_expr = tuple_new(3);
	if (symbol_equal_symbol(ti, dot_sym))
	  tuple_set(dot_expr, 0, op_dot_sym);
	else if (symbol_equal_symbol(ti, nobinddot_sym))
	  tuple_set(dot_expr, 0, op_nobinddot_sym);
	else // don't!
	  assert(0);
	tuple_set(dot_expr, 1, fcc);
      }
      else
	rha_error("dot/nobinddot must have a lhs.\n");
      fcc = 0;  // reset the function call collector
      with_dot = true;  // since we have seen a dot
    }
    else if (fcc == 0) {
      // start a new function call or symbol and finish the last dot
      fcc = resolve_prules(env, ti);
      // if there is a dot, then finish it
      if (with_dot) {
	tuple_set(dot_expr, 2, fcc);
	fcc = dot_expr;
	with_dot = false;
      }
    }
    else if (iscallof(group_sym, ti) || iscallof(tuplefy_sym, ti)) {
      tuple_set(ti, 0, fcc);
      for (int j=1; j<tuple_length(ti); j++)
	tuple_set(ti, j, resolve_prules(env, tuple_get(ti, j)));
      fcc = ti;
    }
    else 
      rha_error("resolve_function_calls_and_dots: can't parse function calls and dots.\n");
  }
  if (with_dot) {
    // finish dot expression
    if (!fcc)
      rha_error("dots must have rhs.\n");
    tuple_set(dot_expr, 2, fcc);
    fcc = dot_expr;
  }
  // did we construct nothing?
  assert(fcc);  // otherwise resolve_function_calls_and_dots shouldn't be called
  //fprint(stdout, "resolve_function_calls_and_dots (final): %o\n", fcc);
  return fcc;
}
