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
#include "rha_types.h"
#include "messages.h"

#include "eval.h"
#include "tuple_fn.h"
#include "symbol_fn.h"
#include "list_fn.h"

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


addr_t addr_fn(object_t o)
{
  return (addr_t) o;
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

/**********************************************************************
 * 
 * Printing objects
 *
 **********************************************************************/

static void _print_ptype(int_t p)
{
  static char *ptype_names[] = {
    "NONE", "SYMBOL", "OBJECT", "INT", "BOOL",
    "TUPLE", "FN", "REAL", "MAT", "STRING", "ADDR",
    "LIST"
  };
  static int maxptype = 11;

  if (p <= maxptype)
    printf(ptype_names[p]);
  else
    printf("%d", p);
}

void print_fn(object_t o)
     // right now, this is a big switch, but eventually, this should
     // be done via symbol lookup :)
{
  if (!o) {
    fprintf(stdout, "NULL (which means: evaluation failed)\n");
  }
  else if (o==void_obj) {
    fprintf(stdout, "void");
  }
  else {
    switch (ptype(o)) {
    case INT_T: fprintf(stdout, "%d", o->raw.i); break;
    case SYMBOL_T: {
      symbol_t s = UNWRAP_SYMBOL(o);
      fprintf(stdout, "%s (sym%d)", symbol_name(s), s); 
      break;
    }
    case STRING_T: {
      string_t s = UNWRAP_PTR(STRING_T, o);
      fprintf(stdout, "\"%s\"", s); 
      break;
    }
    case REAL_T: fprintf(stdout, "%f", o->raw.d); break;
    case TUPLE_T: {
      tuple_t t = UNWRAP_PTR(TUPLE_T, o);
      fprintf(stdout, "(");
      for(int i = 0; i < tuple_len(t); i++) {
	if (i > 1) fprintf(stdout, ", ");
	print_fn(tuple_get(t, i));
      }
      fprintf(stdout, ")");
      break;
    }
    case LIST_T: {
      list_t l = UNWRAP_PTR(LIST_T, o);
      list_it i;
      int c; 

      printf("[");
      for(list_begin(l, &i), c = 0; !list_done(&i); list_next(&i), c++) {
	if(c > 0)
	  printf(", ");
	print_fn(list_get(&i));
      }
      printf("]");
      break;
    }
    case FN_T: {
      fn_t f = UNWRAP_PTR(FN_T, o);
      fprintf(stdout, "[ fn narg=%d, ", f->narg);
      for(int i = 0; i < f->narg; i++) {
	if (i > 1)
	  printf(", ");
	_print_ptype(f->argtypes[i]);
      }
      break;
    }
    default:
      fprintf(stdout, "[ ptype=%d, raw=%8p\n ]", ptype(o), o->raw.p);
    }
  }
}

