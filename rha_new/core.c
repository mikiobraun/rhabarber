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
#include "list_fn.h"


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
 * arithmetics
 *
 *************************************************************/

object_t plus_fn(object_t a, object_t b)
{
  real_t x, y;
  switch (ptype(a)) {
  case INT_T: x = (real_t) UNWRAP_INT(a); break;
  case REAL_T: x = UNWRAP_REAL(a); break;
  default: rha_error("can't add those types");
  }
  return WRAP_REAL(x+y);
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



/**********************************************************************
 * 
 * Helper functions for prules
 *
 **********************************************************************/





/* 
   tools for programming prules
*/



tuple_t resolve_prefix_prule(list_t parsetree, symbol_t fun_sym)
// fs  the function symbol (of the function to be called)
// which is also the prule symbol
// e.g. \(17+42)    for the backslash
{
  // the prule symbol must be the first in the parsetree tuple
  int llen = list_len(parsetree);
  if (llen == 1) {
    // e.g. return or break (to finish a function, returning nothing)
    return tuple_make(1, WRAP_SYMBOL(fun_sym));
  }
  else if (llen > 1) {
    // e.g. return 17
    // replace the prefix symbol by  the function symbol
    tuple_t t = list_to_tuple(parsetree);
    tuple_set(t, 0, WRAP_SYMBOL(fun_sym));
    return t;
  }
  else
    rha_error("resolve_prefix_prule: parse error.\n");
  // never reach this point
  assert(1==0);
}


tuple_t resolve_infix_prule(list_t parsetree, symbol_t prule_sym, symbol_t fun_sym, bool_t left_binding)
// a + (b + c)
{
  // a generic infix prule which can deal with all infix operators
  // let's manipulate the parsetree
  list_t lhs, rhs;
  if (left_binding) {
    lhs = parsetree;
    rhs = list_chop_last(lhs, prule_sym);
  }
  else {
    rhs = parsetree;
    lhs = list_chop_first(rhs, prule_sym);
  }

  if ((list_len(lhs)==0) || (list_len(rhs)==0))
    rha_error("resolve_infix_prule: infix requires a nonempty lhs and rhs.");
  tuple_t t = tuple_new(3);
  tuple_set(t, 0, WRAP_SYMBOL(fun_sym));
  tuple_set(t, 1, WRAP_PTR(LIST_T, list_proto, lhs));
  tuple_set(t, 2, WRAP_PTR(LIST_T, list_proto, rhs));
  return t;
}


#define IGNORE
#ifndef IGNORE

tuple_t precall(int narg, ...)
{
  va_list ap;
  tuple_t t = tuple_new(narg+1);
  va_start(ap, narg);
  for (int i = 0; i < narg; i++)
    tuple_set(t, i+1, va_arg(ap, object_t));
  va_end(ap);

  tuple_set(t, 0, WRAP_SYMBOL(rounded_sym));

  return t;
}


object_t precall1(object_t f, object_t arg1)
// this will create:
//     (f (rounded_sym arg1))
// this expression will be converted in a function call by resolve_prules
{
  return tuple_make(2, f, tuple_make(2, rounded_sym, arg1));
}


object_t precall2(object_t f, object_t arg1, object_t arg2)
// this will create:
//     (f (rounded arg1 arg2))
// this expression will be converted in a function call by resolve_prules
{
  return tuple_make(2, f, tuple_make(3, rounded_sym, arg1, arg2));
}

static list_tr cmp_list = 0; // a list of comparison prule symbols

BUILTIN(resolve_cmp_prule)  // the only arg we are using is 'in'
// resolve stuff like a == b == c < d
{
  if (!cmp_list) {
    // make a list of the comparison prule symbol
    cmp_list = list_new();
    list_append(cmp_list, less_sym);
    list_append(cmp_list, lessequal_sym);
    list_append(cmp_list, greater_sym);
    list_append(cmp_list, greaterequal_sym);
    list_append(cmp_list, equal_sym);
    list_append(cmp_list, notequal_sym);
  }
  // chop the parsetree into pieces
  assert(tuple_length(in)==3);
  tuple_tr parsetreetuple = tuple_get(in, 2);
  list_tr parsetree = tuple_to_list(parsetreetuple);
  printdebug("%o\n", parsetree);
  object_t lhs = list_chop_first_list(parsetree, cmp_list);
  object_t rhs = list_chop_first_list(parsetree, cmp_list);
  int op_pos = list_length(lhs);
  // add the 'op' string to the symbol
  symbol_tr old = tuple_get(parsetreetuple, op_pos);
  symbol_tr op = symbol_new(string_plus_string("op", symbol_name(old)));
  lhs = list_solidify(lhs);
  op_pos += 1 + list_length(rhs);
  rhs = list_solidify(rhs);
  object_t result = 0;
  if (list_length(parsetree)==0) {
    result = tuple_make(3, op, lhs, rhs);
  }
  else {
    // there are more cmp-operators to deal with
    result = precall2(op, lhs, rhs);
    do {
      old = tuple_get(parsetreetuple, op_pos);
      op = symbol_new(string_plus_string("op", symbol_name(old)));
      lhs = rhs;
      rhs = list_chop_first_list(parsetree, cmp_list);
      op_pos += 1 + list_length(rhs);
      rhs = list_solidify(rhs);
      if (list_length(parsetree)==0) {
	result = tuple_make(3, op_and_sym, result, precall2(op, lhs, rhs));
	break;
      }
      else {
	// accumulate
	result = precall2(op_and_sym, result, precall2(op, lhs, rhs));
      }
    } while (true);  // this loop is terminated by "break"
  }

  return result;
}


static list_tr incdec_list = 0; // a list with ++ and --

BUILTIN(resolve_incdec_prule)
{
  if (!incdec_list) {
    // make a list of the incdec prule symbols
    incdec_list = list_new();
    list_append(incdec_list, plusplus_sym);
    list_append(incdec_list, minusminus_sym);
  }
  CHECK_TYPE(tuple, in);
  assert(tuple_length(in) == 3);
  // let's discuss plusplus (minusminus is the same)
  // plusplus_sym must be at the first and/or the last position of the parsetree
  tuple_tr parsetree = tuple_get(in, 2);
  int tlen = tuple_length(parsetree);
  assert(tlen > 1);   // at least ++x or x++, but also ++x++
  list_tr rhs = tuple_to_list(parsetree);
  list_tr lhs = list_chop_first_list(rhs, incdec_list);
  int op_pos = list_length(lhs);
  symbol_tr op = 0;
  object_t result = 0;
  if (op_pos == 0) {
    // (1) at least one operator is at the first entry
    op = tuple_get(parsetree, op_pos);
    result = tuple_make(2, preplusplus_sym, list_solidify(rhs));
    if (symbol_equal_symbol(op, minusminus_sym)) tuple_set(result, 0, preminusminus_sym);
  }
  else if (op_pos == tlen-1) {
    // (2) a single operator at the last entry
    op = tuple_get(parsetree, op_pos);
    result = tuple_make(2, postplusplus_sym, list_solidify(lhs));
    if (symbol_equal_symbol(op, minusminus_sym)) tuple_set(result, 0, postminusminus_sym);
  }
  else {
    rha_error("resolve_incdec_prule: ++ or -- are only prefix/praefix operatoren");
  }
  return result;
}

static list_tr assign_list = 0; // a list with = += -= *= /=

BUILTIN(resolve_assign_prule)
// resolves the right-most assignment (could be = += -= *= /=)
{
  if (!assign_list) {
    // make a list of the assign prule symbols
    assign_list = list_new();
    list_append(assign_list, assign_sym);
    list_append(assign_list, plusassign_sym);
    list_append(assign_list, minusassign_sym);
    list_append(assign_list, timesassign_sym);
    list_append(assign_list, divideassign_sym);
  }
  CHECK_TYPE(tuple, in);
  assert(tuple_length(in) == 3);
  // let's find the first appearance (left-most) of an assignment
  tuple_tr t = tuple_get(in, 2);
  int tlen = tuple_length(t);
  for (int i=0; i<tlen; i++) {
    object_t obj = tuple_get(t, i);
    if (HAS_TYPE(symbol, obj)) {
      int j = 0;
      symbol_tr s = 0;
      list_foreach(s, assign_list) {
	j++;
	if (symbol_equal_symbol(s, obj)) {
	  // note that we always put the op_assign_sym as the function to call
	  tuple_set(in, 1, s);
	  object_t result = resolve_infix_prule(in, op_assign_sym, RIGHT_BIND);
	  switch (j) {
	  case 1: s = 0; break;  // plain assign
	  case 2: s = op_plus_sym; break;
	  case 3: s = op_minus_sym; break;
	  case 4: s = op_times_sym; break;
	  case 5: s = op_divide_sym; break;
	  }
	  if (s) 
	    tuple_set(result, 2, precall2(s, tuple_get(result, 1),
					  tuple_get(result, 2)));
	  // note that assign has three inputs:
	  //   scope, symbol, value
	  result = tuple_make(4, tuple_get(result, 0),
			      local_sym, 
			      tuple_get(result, 1),
			      tuple_get(result, 2));
	  // note that stuff like:
	  //     a.b = 17
	  // becomes
          //     assign_fn(a, b, 17)
	  // find the right most dot on the LHS
	  object_t LHS = tuple_get(result, 2);
	  if (HAS_TYPE(tuple, LHS)) {
	    int LHSlen = tuple_length(LHS);
	    for (int j=LHSlen-1; j>=0; j--) {
	      object_t LHSj = tuple_get(LHS, j);
	      if (HAS_TYPE(symbol, LHSj) 
		  && symbol_equal_symbol(LHSj, dot_sym))
		{
		  // split the tuple into two
		  tuple_tr scope = tuple_new(j);
		  tuple_tr var = tuple_new(LHSlen-j-1);
		  for (int k=0; k<j; k++)
		    tuple_set(scope, k, tuple_get(LHS, k));
		  for (int k=j+1; k<LHSlen; k++)
		    tuple_set(var, k-j-1, tuple_get(LHS, k));
		  // replace the default scope and the var
		  tuple_set(result, 1, scope);
		  tuple_set(result, 2, var);
		  break;
		}
	    }
	  }
	  return result;
	}
      }
    }
  }
  // never reach this point
  rha_error("resolve_assign_prule called without assign symbol.\n");
  return 0;
}


/* object_t check_and_extract_condition(object_t cond) */
/* { */
/*   // the condition must be only one grouped element */
/*   if (HAS_TYPE(list, cond) && list_length(cond) == 1) */
/*     cond = list_solidify(cond); */
/*     if (!iscallof(group_sym, cond) || tuple_length(cond) != 2) */
/*       rha_error("The condition of 'if' must be one expr enclosed in brackets.\n"); */
/*     cond = tuple_get(cond, 1); // get rid of the brackets */
  
/* } */


object_t resolve_ctrl_prule(tuple_tr in)
// if (cond) code
// if (cond) code else code_else
// try code catch (x) code
// while (cond) code
// for (init, check, update) code
// for (x in y) code
// fn () code                // functions
// fn (x) code
// fn (x, y) code
// ...
// macro () code             // macro (like functions, but args are not evaluated)
// macro (x) code
// macro (x, y) code
// ...
// prule (priority, parsetree_var) code    // parse rule
{
  assert(tuple_length(in)==3);
  object_t t = tuple_get(in, 2);
  symbol_tr s = tuple_get(in, 1);
  assert(HAS_TYPE(symbol, s));
  CHECK_TYPE(tuple, t);
  list_tr code = tuple_to_list(t);
  // all above prules have at least three parts
  if (list_length(code) < 3) rha_error("resolve_ctrl_prule error 1.\n");
  object_t o = list_pop(code);
  // all above prules start with a keyword
  if (!HAS_TYPE(symbol, o) || !symbol_equal_symbol(o, s))
    rha_error("resolve_ctrl_prule error 2.\n");
  object_t cond = list_pop(code);
  
  // extra stuff for various symbols
  object_t result = 0;
  if (symbol_equal_symbol(s, if_sym)) {
    // if (cond) code_then
    // if (cond) code_then else code
    if (!iscallof(tuplefy_sym, cond) || tuple_length(cond) != 2)
      rha_error("Condition for 'if' must be a single expression in brackets.\n");
    cond = tuple_get(cond, 1);
    // check for else
    int codelen = list_length(code);
    list_tr code_then = list_chop_matching(code, if_sym, else_sym);
    if (codelen == list_length(code_then)+1)  // there was no code after "else"
      rha_error("'else' must be followed by code.\n");

    if (list_length(code) == 0)
      // no 'else'
      result = tuple_make(3, op_if_sym, cond, 
			  list_solidify(code_then));
    else
      // with 'else'
      result = tuple_make(4, op_if_sym, cond, 
			  list_solidify(code_then),
			  list_solidify(code));
  }
  else if (symbol_equal_symbol(s, try_sym)) {
    // try tryblock catch (catchvar) catchblock
    list_prepend(code, cond);   // there is no 'cond' part here, so put it back
    list_tr tryblock = list_chop_matching(code, try_sym, catch_sym);
    if (list_length(tryblock)==0)
      rha_error("try-block is missing.\n");
    int lcode = list_length(code);
    if (lcode == 0) 
      rha_error("catch-block is missing.\n");
    else if (lcode == 1) 
      rha_error("catch-block must have catch variable and catch-code.\n");
    object_t catchvar = list_pop(code);  // the catch variable
    if (!iscallof(tuplefy_sym, catchvar) || tuple_length(catchvar) != 2)
      rha_error("'catch' must be followed by a single expression (catch variable) in brackets.\n");
    catchvar = tuple_get(catchvar, 1);
    result = tuple_make(4, op_try_sym,
			list_solidify(tryblock),
			catchvar,
			list_solidify(code));
  }
  else if (symbol_equal_symbol(s, while_sym)) {
    // while (cond) code
    if (!iscallof(tuplefy_sym, cond) || tuple_length(cond) != 2)
      rha_error("Condition for 'while' must be a single expression in brackets.\n");
    cond = tuple_get(cond, 1);
    // that's it
    result = tuple_make(3, op_while_sym, cond, list_solidify(code));
  }
  else if (symbol_equal_symbol(s, for_sym)) {
    // for (init, check, update) code
    // for (element in container) code
    // note that 'cond' contains the parameters of 'for'
    if (iscallof(tuplefy_sym, cond) && tuple_length(cond)==4) {
      object_t init = tuple_get(cond, 1);
      object_t check = tuple_get(cond, 2);
      object_t update = tuple_get(cond, 3);
      result = tuple_make(5, op_for_cstyle_sym, init, check, update, 
			  list_solidify(code));
    }
    else if (iscallof(tuplefy_sym, cond) && tuple_length(cond)==2) {
      // extract element and container
      cond = tuple_get(cond, 1);
      if (tuple_length(cond)>=3) {
	list_tr condlist = tuple_to_list(cond);
	object_t t0 = list_pop(condlist);
	object_t t1 = list_pop(condlist);
	if (HAS_TYPE(symbol, t1) && symbol_equal_symbol(in_sym, t1))
	  result = tuple_make(4, op_for_iterator_sym, t0, 
			      list_solidify(condlist), list_solidify(code));
      }
    }
    if (!result) rha_error("'for' requires special forms for its arg.\n");
  }
  else if (symbol_equal_symbol(s, fn_sym) || symbol_equal_symbol(s, macro_sym)) {
    // fn () code
    // fn (x) code
    // fn (x, y) code
    // macro () code
    // macro (x) code
    // macro (x, y) code

    // note that 'cond' are for 'fn' and 'macro' their args
    // do some checks on the args
    if (!iscallof(tuplefy_sym, cond))
      rha_error("args must be a tuple of symbols or a single grouped symbol.\n");
    if (symbol_equal_symbol(s, fn_sym)) 
      tuple_set(cond, 0, op_fn_sym);
    else  
      tuple_set(cond, 0, op_macro_sym);
    result = tuple_to_list(cond);
    list_append(result, list_solidify(code));
    result = list_solidify(result);
  }
  else if (symbol_equal_symbol(s, prule_sym)) {
    // prule (priority, parsetree_var) code
    // note that 'cond' for 'prule' is a pair which contains a real number 
    // and a symbol with which we can access the parsetree
    if (iscallof(tuplefy_sym, cond)) {
      if (tuple_length(cond)!=3) 
	rha_error("prule has a special form: prule (priority, parsetree_var) code.\n");
    }
    else 
      rha_error("prule has a special form: prule (priority, parsetree_var) code.\n");
    tuple_set(cond, 0, op_prule_sym);
    result = tuple_to_list(cond);
    list_append(result, list_solidify(code));
    result = list_solidify(result);
  }

  if (!result) rha_error("resolve_crt_prule error.\n");
  return result;
}

object_t resolve_try_catch_prule(tuple_tr in) {
  return 0;
}

/*
  Program logic
*/ 


/*
 * lazy_or and lazy_and macro
 */

BUILTIN(b_lazy_or)
{
  int nin = tuple_length(in);
  if (nin == 3) {
    fprint(stdout, "lazy or\n");
    object_t lhs = tuple_get(in, 1);
    object_t rhs = tuple_get(in, 2);
    lhs = eval(env, lhs);
    if (HAS_TYPE(bool, lhs))
      if (bool_get(lhs))
	return bool_new(true);
      else {
	rhs = eval(env, rhs);
	if (HAS_TYPE(bool, rhs))
	  if (bool_get(rhs))
	    return bool_new(true);
	  else
	    return bool_new(false);
	else
	  rha_error("rhs of 'op||' must eval to bool.\n");
      }
    else
      rha_error("lhs of 'op||' must eval to bool.\n");
  }
  // otherwise wrong number of args
  rha_error("'op||' must be called with exactly two args.\n");
  return 0;
}

BUILTIN(b_lazy_and)
{
  int nin = tuple_length(in);
  if (nin == 3) {
    fprint(stdout, "lazy and\n");
    object_t lhs = tuple_get(in, 1);
    object_t rhs = tuple_get(in, 2);
    lhs = eval(env, lhs);
    if (HAS_TYPE(bool, lhs))
      if (!bool_get(lhs))
	return bool_new(false);
      else {
	rhs = eval(env, rhs);
	if (HAS_TYPE(bool, rhs))
	  if (bool_get(rhs))
	    return bool_new(true);
	  else
	    return bool_new(false);
	else
	  rha_error("rhs of 'op&&' must eval to bool.\n");
      }
    else
      rha_error("lhs of 'op&&' must eval to bool.\n");
  }
  // otherwise wrong number of args
  rha_error("'op&&' must be called with exactly two args.\n");
  return 0;
}


#endif
