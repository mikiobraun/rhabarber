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


object_t fn_fn(object_t this, tuple_t argnames, object_t fnbody)
{
  // defines a new rhabarber function
  object_t f = new();
  assign(f, scope_sym, this);
  assign(f, argnames_sym, WRAP_PTR(TUPLE_T, tuple_proto, argnames));
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
  // thow_fn does not return!
  frame_jump(FUNCTION_FRAME, retval);
  // never reaches this point
}


void deliver_fn(object_t retval)
{
  // thow_fn does not return!
  frame_jump(BLOCK_FRAME, retval);
  // never reaches this point
}


void break_fn(object_t retval)
{
  // thow_fn does not return!
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
  object_t res = void_obj; // must be initialized for "cond==false"
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
  rha_error("not yet");
  return 0;
}


object_t for_fn(object_t this, symbol_t var, object_t container, object_t code)
{
  rha_error("not yet!");
  return 0;
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
  fprint(stdout, "--loading \"%s\"\n", fname);
  object_t p = parse_file(root, fname);
  if (p) {
    return eval(root, p);
  }
  else
    fprintf(stdout, "parse returned ZERO\n");
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

