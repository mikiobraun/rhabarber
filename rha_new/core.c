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
#include "messages.h"

#include "eval.h"
#include "tuple_fn.h"
#include "symbol_fn.h"
#include "string_fn.h"
#include "list_fn.h"
#include "utils.h"
#include "alloc.h"
#include "parse.h"


object_t proxy_fn(object_t this, symbol_t s)
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
  object_t obj = new();
  // create proxy function
  assign(obj, hasproxy_sym, fn_fn(this, tuple_new(0), WRAP_SYMBOL(s)));
  return obj;
}

object_t fn_fn(object_t this, tuple_t argnames, object_t fnbody)
{
  // defines a new rhabarber function
  object_t f = new();
  assign(f, scope_sym, this);
  assign(f, argnames_sym, WRAP_PTR(TUPLE_T, argnames));
  assign(f, fnbody_sym, fnbody);
  return f;
}

object_t prule_fn(object_t this, tuple_t argnames, object_t fnbody, real_t priority)
{
  // defines a new prule, which is:
  // * a function with the parsetree as the single argument
  // * an object with a 'priority' slot
  if (tuple_len(argnames) != 1)
    rha_error("prules must have only a single argument for the parsetree");
  object_t p = fn_fn(this, argnames, fnbody);
  assign(p, priority_sym, WRAP_REAL(priority));
  return p;
}

object_t if_fn(object_t this, bool_t cond, object_t then_code, object_t else_code)
{
  // we assume that the condition is already evaluated
  if (cond)
    return eval(this, then_code);
  else
    return eval(this, else_code);
}


void return_fn(object_t retval)
{
  // note that 'return_fn' also added a FUNCTION_FRAME to the frame
  // stack, which we have to ignore in the following 'frame_jump',
  // thus
  frame_tos--;

  // throw_fn does not return!
  frame_jump(FUNCTION_FRAME, retval);
  // never reaches this point
}


void deliver_fn(object_t retval)
{
  // note that 'deliver_fn' also added a FUNCTION_FRAME to the frame
  // stack, which we have to ignore in the following 'frame_jump',
  // thus
  frame_tos--;

  // throw_fn does not return!
  frame_jump(BLOCK_FRAME, retval);
  // never reaches this point
}


void break_fn(object_t retval)
{
  // note that 'break_fn' also added a FUNCTION_FRAME to the frame
  // stack, which we have to ignore in the following 'frame_jump',
  // thus
  frame_tos--;

  // throw_fn does not return!
  frame_jump(LOOP_FRAME, retval);
  // never reaches this point
}


void throw_fn(object_t excp)
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


object_t while_fn(object_t this, object_t cond, object_t body)
{
  object_t res = 0; // must be initialized for "cond==false"
  begin_frame(LOOP_FRAME)
    while (1) {
      if (UNWRAP_BOOL(eval(this, cond)))
	res = eval(this, body);
      else
	break;
    }
  end_frame(res);
  return res;
}


object_t try_fn(object_t this, object_t tryblock, symbol_t catchvar, object_t catchblock)
{
  object_t excp = 0;
  object_t result = 0;
  try {
    result = eval(this, tryblock);
  }
  catch (excp) {
    assign(this, catchvar, excp);
    result = eval(this, catchblock);
  }
  return result;
}


object_t for_fn(object_t this, symbol_t var, object_t container, object_t body)
{
  object_t res = 0;
  object_t iter = callslot(container, iter_sym, 0);
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

object_t run_fn(object_t root, string_t fname)
{
  object_t p = parse_file(root, fname);
  if (p) { // otherwise, input was empty
    return eval(root, p);
  }
  return 0;
}

/************************************************************
 *
 * Lazy logic
 *
 *************************************************************/

bool_t and_fn(bool_t a, bool_t b)
{
  if (!a) return false;
  if (!b) return false;
  return true;
}

bool_t or_fn(bool_t a, bool_t b)
{
  if (a) return true;
  if (b) return true;
  return false;
}

/************************************************************
 *
 * For slicing and typing and complex literals
 *
 *************************************************************/


object_t literal(object_t env, list_t parsetree)
{
  // this is the default complex literal handler
  // for now we simply generate a list by getting rid of all COMMA and
  // by calling resolve on each sublist
  list_t sink = list_new();
  list_t part = list_new();
  object_t head = 0;
  while ((head = list_popfirst(parsetree))) {
    if (is_symbol(comma_sym, head)) {
      if (list_len(part)>0) {
	// split here and ignore the comma
	list_append(sink, resolve_list_by_prules(env, part));
	part = list_new();
      }
      continue;
    }
    else if (is_symbol(semicolon_sym, head)) {
      // this is not allowed
      rha_error("(parsing) in a list literal is no semicolon allowed");
    }
    else {
      list_append(part, head);
    }
  }
  if (list_len(part)>0)
    list_append(sink, resolve_list_by_prules(env, part));

  // finally evaluate all entries
  list_t source = sink;
  sink = list_new();
  while ((head = list_popfirst(source)))
    list_append(sink, eval(env, head));
  
  // and wrap it up!

  return WRAP_PTR(LIST_T, sink);  // a list!
}
