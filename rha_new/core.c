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
#include "rha_types.h"


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


void throw_fn(object_t excp);
{
  // thow_fn does not return!
  throw(excp);
  // never reaches this point
}


object_t do_fn(object_t this, tuple_t code)
{
  // 'do_fn' returns 'void' or a return value ('return 17')
  // 'do_fn' doesn't open a new scope
  object_t res = void_obj;
  int tlen = tuple_len(code);
  begin_frame(BLOCK_FRAME) {
    // evaluate all
    for (int i = 0; i<tlen; i++) {
      eval(this, tuple_get(code, i));
    }
  }
  end_frame(res);
  // return the result, which might be 'void'
  return res;
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
      if (UNWRAP_BOOL(eval(env, tuple_get(in, 1))))
	res = eval(env, body);
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


addr_t addr_fn(object_t o)
{
  return (addr_t) &o;
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

double toc_fn
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  tv.tv_sec -= tic_saved_sec;
  double now = tv.tv_sec + tv.tv_usec*1e-6;

  return now - tic_saved_time;
}
