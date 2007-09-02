/*
 * core - core functions
 
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include "core.h"

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "rha_types.h"
#include "rha_parser.h"
#include "messages.h"

#include "eval.h"
#include "tuple_fn.h"
#include "symbol_fn.h"
#include "string_fn.h"
#include "list_fn.h"
#include "utils.h"
#include "alloc.h"
#include "parse.h"


any_t proxy_fn(any_t this, symbol_t s)
{
  // note that this function can't be implemented fully in rhabarber itself!!!
  // failing attempt:
  //
  // proxy = fn (s) {
  //   y = new();
  //   y.proxy = fn () lookup(parent.this, s);
  //   return y;
  // }
  //
  // the problem is that after adding the slot 'proxy' to 'y', we can
  // not return 'y', since it is not accessible anymore.
  any_t obj = new();
  // create proxy function
  assign(obj, hasproxy_sym, fn_fn(this, tuple_new(0), WRAP_SYMBOL(s)));
  return obj;
}

any_t create_pattern(int_t narg, ...)
{
  va_list ap;
  va_start(ap, narg);
  tuple_t args = tuple_new(narg);
  for (int i = 0; i < narg; i++)
    tuple_set(args, i,va_arg(ap, any_t));
  va_end(ap);
  return vcreate_pattern(args);
}

any_t vcreate_pattern(tuple_t args)
{
  // what is a pattern?
  // x
  // int:x
  // true
  // 1
  // 1.0
  // anything that can be compared with other stuff
  any_t theliteral = 0;
  any_t thetype = 0;
  int narg = tuple_len(args);
  if (narg > 0) {
    // check the type of the first argument by hand
    theliteral = tuple_get(args, 0);
    if (theliteral 
	&& ptype(theliteral) != SYMBOL_T
	&& ptype(theliteral) != BOOL_T
	&& ptype(theliteral) != INT_T
	&& ptype(theliteral) != REAL_T)
      rha_error("(pattern) the arg must be symbol, bool, int or real");
  }
  if (narg > 1) {
    if (theliteral && ptype(theliteral) != SYMBOL_T)
      rha_error("(pattern) only symbols can be typed");
    thetype = tuple_get(args, 1);
    // do type checking here by hand!
    if (thetype && !has(thetype, check_sym))
      rha_error("(pattern) the type does not have slot 'check'");
  }
  if (narg > 2) {
    rha_error("(pattern) can't create pattern");
  }
  // note that both 'thetype' and 'theliteral' could be void.  this
  // happens for builtin function that only have ANY_T arguments
  // then the resulting 'pattern' is the empty object
  any_t pattern = new();
  assign(pattern, parent_sym, pattern_proto);
  if (thetype)
    assign(pattern, symbol_new("patterntype"), thetype);
  if (theliteral)
    assign(pattern, symbol_new("patternliteral"), theliteral);
  return pattern;
}


any_t create_fn_data_entry(any_t env, tuple_t signature, any_t fnbody)
{
  // check for ellipsis symbol
  // it is only allowed at the end of a signature
  list_t signature_l = list_new();
  int i, tlen = tuple_len(signature);
  for (i = 0; i < tuple_len(signature); i++) {
    any_t pattern = tuple_get(signature, i);
    any_t theliteral = lookup(pattern, symbol_new("patternliteral"));
    if (theliteral
	&& ptype(theliteral) == SYMBOL_T
	&& UNWRAP_SYMBOL(theliteral) == symbol_new("..."))
      break;
    list_append(signature_l, pattern);
  }
  bool_t varargs;
  if (i < tlen-1) {
    rha_error("ellipsis (...) is only allowed at the end of a signature");
  }
  else if (i == tlen-1) {
    signature = list_to_tuple(signature_l); // removes the ellipsis
    varargs = true;
  }
  else {
    varargs = false;
  }

  // built fn_data entry
  any_t entry = new();
  assign(entry, signature_sym, WRAP_PTR(TUPLE_T, signature));
  if (env)
    assign(entry, scope_sym, env);
  assign(entry, fnbody_sym, fnbody);
  assign(entry, varargs_sym, WRAP_BOOL(varargs));
  assign(entry, parent_sym, implementation_proto);
  return entry;
}

any_t create_fn_data(any_t env, tuple_t signature, any_t fnbody)
{
  // note the order
  list_t fn_data_l = list_new();
  list_append(fn_data_l, create_fn_data_entry(env, signature, fnbody));
  any_t fn_data = WRAP_PTR(LIST_T, fn_data_l);
  assign(fn_data, parent_sym, fn_data_proto); // this makes sure that
					      // it can be converted
					      // to string
  return fn_data;
}

any_t create_function(any_t fn_data)
{
  // defines a new function
  any_t f = new();
  assign(f, fn_data_sym, fn_data);
  return f;
}

any_t fn_fn(any_t env, tuple_t signature, any_t fnbody)
{
  return create_function(create_fn_data(env, signature, fnbody));
}

any_t macro_fn(tuple_t argnames, any_t fnbody)
{
  any_t m = new();
  assign(m, ismacro_sym, WRAP_BOOL(true));
  assign(m, argnames_sym, WRAP_PTR(TUPLE_T, argnames));
  assign(m, fnbody_sym, fnbody);
  return m;
}

any_t prule_fn(any_t this, tuple_t argnames, any_t fnbody, real_t priority)
{
  assert(1==0);  // is this ever used???

  // defines a new prule, which is:
  // * a function with the parsetree as the single argument
  // * an object with a 'priority' slot
  if (tuple_len(argnames) != 1)
    rha_error("prules must have only a single argument for the parsetree");
  any_t p = fn_fn(this, argnames, fnbody);
  assign(p, priority_sym, WRAP_REAL(priority));
  return p;
}

tuple_t map_fn(any_t this, any_t f, tuple_t t)
{
  // applies function f to all entries in the tuple
  // each entry is assumed to be a single entry itself or a tuple
  int tlen = tuple_len(t);
  tuple_t sink_t = tuple_new(tlen);
  tuple_t call_t = 0;
  for (int i = 0; i < tlen; i++) {
    any_t entry = tuple_get(t, i);
    if (ptype(entry) == TUPLE_T) {
      list_t l = tuple_to_list(UNWRAP_PTR(TUPLE_T, entry));
      list_prepend(l, quoted(f));
      call_t = list_to_tuple(l);
    }
    else {
      call_t = tuple_new(2);
      tuple_set(call_t, 0, quoted(f));
      tuple_set(call_t, 1, entry);
    }
    tuple_set(sink_t, i, eval(this, WRAP_PTR(TUPLE_T, call_t)));
  }
  return sink_t;
}

any_t if_fn(any_t this, bool_t cond, any_t then_code, any_t else_code)
{
  // we assume that the condition is already evaluated
  if (cond)
    return eval(this, then_code);
  else
    return eval(this, else_code);
}


void return_fn(any_t retval)
{
  frame_jump(FUNCTION_FRAME, retval);
  // never reaches this point
}


void deliver_fn(any_t retval)
{
  frame_jump(BLOCK_FRAME, retval);
  // never reaches this point
}


void break_fn(any_t retval)
{
  frame_jump(LOOP_FRAME, retval);
  // never reaches this point
}


void throw_fn(any_t excp)
{
  // thow_fn does not return!
  throw(excp);
  // never reaches this point
}



void exit_fn(int_t exit_code)
{
  saybye();
  exit(exit_code); 
}


any_t while_fn(any_t this, any_t cond, any_t body)
{
  any_t res = 0; // must be initialized for "cond==false"
  begin_frame(LOOP_FRAME)
    while (1) {
      any_t cond_o = eval(this, cond);
      if (!cond_o || ptype(cond_o)!=BOOL_T)
	rha_error("(while) condition must generate bool");
      if (UNWRAP_BOOL(cond_o))
	res = eval(this, body);
      else
	break;
    }
  end_frame(res);
  return res;
}


any_t try_fn(any_t this, any_t tryblock, symbol_t catchvar, any_t catchblock)
{
  any_t excp = 0;
  any_t result = 0;
  try {
    result = eval(this, tryblock);
  }
  catch (excp) {
    assign(this, catchvar, excp);
    result = eval(this, catchblock);
  }
  return result;
}


any_t for_fn(any_t this, symbol_t var, any_t container, any_t body)
{
  any_t res = 0;
  any_t iter = callslot(container, iter_sym, 0);
  begin_frame(LOOP_FRAME)
    while (!UNWRAP_BOOL(callslot(iter, done_sym, 0))) {
      assign(this, var, callslot(iter, get_sym, 0));
      res = eval(this, body);
      callslot(iter, next_sym, 0);
    }
  end_frame(res);
  return res;
}




/************************************************************
 *
 * For time measurement purposes
 *
 *************************************************************/

#include <time.h>
#include <sys/time.h>

int tic_saved_sec;
double tic_saved_time;

void tic_fn()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
    
  tic_saved_sec = tv.tv_sec;
  tic_saved_time = tv.tv_usec * 1e-6;
}

real_t toc_fn()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  tv.tv_sec -= tic_saved_sec;
  double now = tv.tv_sec + tv.tv_usec*1e-6;

  return now - tic_saved_time;
}


/************************************************************
 *
 * Importing and running files
 *
 *************************************************************/

any_t split_resolve_and_eval(any_t root, list_t source, string_t context)
{
  // parse split by semicolon (and other tricks)
  list_t l = split_by_semicolon(source);
  
  // resolve and eval each expression
  any_t obj=0, expr=0, value=0, excp=0;
  while ((obj = list_popfirst(l))) {
    assert(ptype(obj) == LIST_T);
    try {
      expr = resolve(root, UNWRAP_PTR(LIST_T, obj));
      value = eval(root, expr);
    }
    catch (excp) {
      if (context)
	excp = excp_new(string_concat(excp_msg(excp), 
				      sprint("\n[%s] %o", context, obj)));
      throw(excp);
    }
  }
  // return the last one
  return value;
}

any_t run_fn(any_t root, string_t fname)
{
  return split_resolve_and_eval(root, rhaparsefile(fname), fname);
}

/************************************************************
 *
 * For slicing and typing and complex literals
 *
 *************************************************************/

any_t colon_fn(any_t a, any_t b)
{
  if (ptype(a) == INT_T && ptype(b) == INT_T) {
    int_t i = UNWRAP_INT(a);
    int_t j = UNWRAP_INT(b);
    // create a tuple with those numbers
    // e.g.   0:4   ->   tuple:[0, 1, 2, 3]
    tuple_t t = 0;
    if (i>j) 
      t = tuple_new(0);
    else {
      t = tuple_new(j-i);
      for (int k=i; k<j; k++)
	tuple_set(t, k-i, WRAP_INT(k));
    }
    return WRAP_PTR(TUPLE_T, t);
  }
  rha_warning("(colon_fn) for non-integers, 'a:b' returns 'b'");
  return b;
}


any_t literal(any_t env, list_t parsetree)
{
  // this is the default complex literal handler
  // for now we simply generate a list by getting rid of all COMMA and
  // by calling resolve on each sublist
  list_t sink = list_new();
  list_t part = list_new();
  any_t head = 0;
  while ((head = list_popfirst(parsetree))) {
    if (is_symbol(symbol_new(","), head)) {
      if (list_len(part)>0) {
	// split here and ignore the comma
	list_append(sink, resolve(env, part));
	part = list_new();
      }
      continue;
    }
    else if (is_symbol(symbol_new(";"), head)) {
      // this is not allowed
      rha_error("(parsing) in a list literal is no semicolon allowed");
    }
    else {
      list_append(part, head);
    }
  }
  if (list_len(part)>0)
    list_append(sink, resolve(env, part));

  // finally evaluate all entries
  list_t source = sink;
  sink = list_new();
  while ((head = list_popfirst(source)))
    list_append(sink, eval(env, head));
  
  // and wrap it up!

  return WRAP_PTR(LIST_T, sink);  // a list!
}
