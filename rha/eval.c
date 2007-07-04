/*
 * eval - the interpreter
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include "eval.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

//#define DEBUG
#include "debug.h"
#include "utils.h"
#include "messages.h"
#include "object.h"
#include "rhavt.h"

#include "plain_tr.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "real_tr.h"
#include "builtin_tr.h"
#include "function_tr.h"
#include "core.h"
#include "overloaded_tr.h"
//#include "method_tr.h"
#include "none_tr.h"
#include "bool_tr.h"
#include "prule.h"

object_t loc(object_t env, object_t expr);
object_t assign(object_t env, object_t scope, object_t sym, object_t rhs);
object_t eval_args(object_t env, object_t expr);
object_t eval_lookupfailed_hook(object_t, object_t);
object_t eval_fctcall_hook(object_t env, object_t tuple);

object_t eval_currentlocation = 0;

#define ENTER     object_t savedloc = eval_currentlocation; eval_currentlocation = expr
#define RETURN(x) do { object_t retval = (x); \
                           eval_currentlocation = savedloc; \
                           return retval; } while(0)


object_t resolve_eval_list(object_t env, list_tr exprlist)
{
  CHECK_TYPE(list, exprlist);
  object_t o = 0;
  object_t expr;
  list_foreach(expr, exprlist) {
    fprint(stdout, "before resolving prules: %o\n", expr);
    o = resolve_prules(env, expr);
    fprint(stdout, "after resolving prules:  %o\n", o);
    o = eval(env, o);
  }
  return o;
}




/* eval - the main evaluation function
 *
 * the following constructs are recognized:
 *
 * symbol                
 * semiclist
 * literals
 * (f 17 42)    function calls
 * \ expr       could be dealt with by usual function calls????
 * 
 * all the rest is handled elsewhere
 * lhs = expr (see assign)
 * expr . symbol
 * expr . expr
 * fn(args) expr
 */


object_t eval(object_t env, object_t expr)
{
  ENTER;
  //print("eval(env = %p, expr = %o)\n", env, expr);

  // symbol
  if(HAS_TYPE(symbol, expr)) {
    // local
    if (symbol_equal_symbol(local_sym, expr))
      return env;
    // all other stuff
    object_t o = rha_lookup(env, expr);
    if (!o) {
      object_t x;  // iter var
      list_foreach(x, path) {
	o = rha_lookup(x, expr);
	if (o) break;
      }
    }
    if (!o) o = eval_lookupfailed_hook(env, expr);
    if (!o) 
      rha_error("eval(): symbol \"%s\" not found!\n", symbol_name(expr));
    RETURN( o );
  }

  // quote, e.g. \a
  else if(iscallof(op_quote_sym, expr)) {
    RETURN( tuple_get(expr, 1) );
  }

  // assignment
  else if(iscallof(op_assign_sym, expr)) {
    object_t scope = tuple_get(expr, 1);
    object_t lhs = tuple_get(expr, 2);
    object_t rhs = tuple_get(expr, 3);
    RETURN( assign(env, scope, lhs, rhs) );
  }

  // dot expression, e.g. a.b
  else if(iscallof(op_dot_sym, expr)) {
    object_t lhs = eval(env, tuple_get(expr, 1));
    object_t rhs = tuple_get(expr, 2);
    if (HAS_TYPE(symbol, rhs)) {
      // do the look up in lhs
      rhs = rha_lookup(lhs, rhs);
      if (!rhs)
	rha_error("No slot with name \"%o\" in object %o.\n", rhs, lhs);
      RETURN( rhs );
    }
    else {
      // we are calling a function!
      assert(HAS_TYPE(tuple, rhs));
      assert(tuple_length(rhs)>0);
      object_t rhs0 = tuple_get(rhs, 0);
      if (!HAS_TYPE(symbol, rhs0))
	rha_error("RHS of dots are not evaluated only looked up.\n");
      // evaluate the symbol in 'lhs'
      tuple_set(rhs, 0, eval(lhs, rhs0));
      // evaluate the args in the local scope
      object_t t = eval_args(env, rhs);
      // call the function in 'lhs' which is 'this'
      RETURN( rha_call(lhs, t) );
    }
    assert(1==0);
  }

  //// question-mark expression
  //else if(iscallof(op_quest_sym, expr)) {
  //  object_t obj = eval(env, tuple_get(expr, 1));
  //  object_t slotname = tuple_get(expr, 2);
  //  if (!HAS_TYPE(symbol, slotname)) {
  //    slotname = eval(env, slotname);
  //    if (!HAS_TYPE(symbol, slotname)) {
  //       rha_error("Rhs of a ? must be symbol or must evaluate to a symbol.\n");
  //    }
  //  }
  //  RETURN( bool_new( object_lookup(obj, slotname) != 0) );
  //}

  //else if(iscallof(op_nobinddot_sym, expr)) {
  //  object_t obj = eval(env, tuple_get(expr, 1));
  //  object_t slotname = tuple_get(expr, 2);
  //  if (!HAS_TYPE(symbol, slotname)) {
  //    slotname = eval(env, slotname);
  //    if (!HAS_TYPE(symbol, slotname)) {
  //      rha_error("Rhs of a ./ must be symbol or must evaluate to a symbol.\n");
  //    }
  //  }
  //  object_t slot = object_lookup(obj, slotname);
  //  if(!slot)
  //    rha_error("No slot with name \"%o\" in object %o.\n", slotname, slot);
  //  RETURN( slot );
  //}

  // fn expression, e.g. fn (x,y) x + y
  else if(iscallof(op_fn_sym, expr)) {
    int tlen = tuple_length(expr);
    RETURN( function_new(expr, tlen - 2, tuple_get(expr, tlen - 1), env) );
  }

  // macro expression, e.g. macro (x,y) x + y
  else if(iscallof(op_macro_sym, expr)) {
    int tlen = tuple_length(expr);
    RETURN( function_new_macro(expr, tlen - 2, tuple_get(expr, tlen - 1), env) );
  }

  // prule expression, e.g. prule (priority, parsetree_var) code
  else if(iscallof(op_prule_sym, expr)) {
    int tlen = tuple_length(expr) - 1;
    object_t expr1 = tuple_get(expr, 1);
    if (!HAS_TYPE(real, expr1))
      rha_error("prule must have a real number for the priority: prula (10.0, pt) code");
    double priority = real_get(expr1);
    // get rid of the priority
    expr = tuple_shiftfirst(expr);
    tuple_set(expr, 0, op_prule_sym);
    RETURN( function_new_prule(expr, tlen - 2, tuple_get(expr, tlen - 1), env, priority) );
  }

  // function call, e.g.  f(x, y, z)
  else if(HAS_TYPE(tuple, expr)) {
    int tlen = tuple_length(expr);
    assert(tlen>0);
    // evaluate the function in the local scope
    object_t f = eval(env, tuple_get(expr, 0));
    if (!f)
      rha_error("Can't evaluate the function\n.");
    tuple_set(expr, 0, f);
    // evaluate the args in the local scope
    object_t t = eval_args(env, expr);
    // call the function
    RETURN( rha_call(env, t) );
  }

  // literals, e.g. 1, 2, "foobar"
  RETURN( expr );
}


object_t eval_args(object_t env, object_t expr)
{
  // evaluate args
  int exprlen = tuple_length(expr);
  assert(exprlen>0);
  tuple_tr t = tuple_new(exprlen);
  tuple_set(t, 0, tuple_get(expr, 0));
  bool ismacro = rha_ismacro(tuple_get(t, 0));
  for (int i = 1; i < exprlen; i++) {
    object_t arg = tuple_get(expr, i);
    if (!ismacro)
      arg = eval(env, arg);
    if (!arg) 
      rha_error("Could not evaluate argument.\n");
    tuple_set(t, i, arg);
  }
  return t;
}


object_t loc(object_t env, object_t expr)
     // loc == look up or create
     // this function is only used for assignments to dot expressions
{
  assert(0==1);  // don't use this function anymore

  printdebug("loc(env = %p, expr = %o)\n", (void*)env, expr);

  // symbol
  if (HAS_TYPE(symbol, expr)) {
    object_t o = rha_lookup(env, expr);
    if(!o) o = object_assign(env, expr, plain_new());
    return o;
  }

  // dot expression
  else if (iscallof(op_dot_sym, expr))  {
    object_t l = loc(env, tuple_get(expr, 1));
    return loc(l, tuple_get(expr, 2));
  }

  // for example, when lhs is a dictionary or tuple.
  else if (HAS_TYPE(tuple, expr)) {
    return eval(env, expr);
  }

  rha_error("Can not lookup or create %o!\n", expr); 
  return 0; /* never reaches this point */
}

// assign to a symbol.
//
// This function checks whether the first character of the symbol is
// '@', then the assignment is local in any case. Otherwise, it might
// replace a slot in one of the parents.
object_t assigntosymbol(object_t env, object_t lhs, object_t rhs)
{
  assert(0==1);  // don't use this function anymore


  string_t str = symbol_name(lhs);
  if (str[0] == '@') {
    // local variable
    return object_assign(env, symbol_new(str+1), rhs);
  }
  else {
    // possibly non-local
    object_t o;
    if(!(o = rha_replace(env, lhs, rhs)))
      return object_assign(env, lhs, rhs);
    else
      return o;
  }

  rha_error("Can not assign %o to %o!\n", rhs, lhs);
  return 0; /* never reaches this point */
}


/* assign - the main assignment function
 *
 * the following constructs are recognized:
 *
 * lhs = rhs
 *
 * Simple assignments:
 *
 *       symbol = rhs
 *       target . symbol = rhs
 *
 * Function-call like assignments:
 *
 *       symbol(args) = rhs
 *       target . symbol(args) =rhs
 *
 * The target itself can be a dotted list of expressions:
 *
 *       target = symbol | expr . symbol
 */

object_t assign(object_t env, object_t scope, object_t lhs, object_t rhs)
// 'scope' and 'rhs' will be evaluated in 'env'
// 'lhs' will be created in 'scope'
//
// there are two cases:
// (i)    x = 17       // assign new symbol
// (ii)   f(x) = 17    // adding a case to a function
{
  printdebug("assign(env = %p, scope = %o, lhs = %o, rhs = %o)\n", 
	     (void*)env, scope, lhs, rhs);

  if (HAS_TYPE(symbol, lhs)) {
    // always local to 'scope'
    scope = eval(env, scope);
    return object_assign(scope, lhs, eval(env, rhs));
  }
  // Function-call like assignments: expr() = rhs
  else if (HAS_TYPE(tuple, lhs)) {
    object_t target = tuple_get(lhs, 0);

    //print("Looking at target %o\n", target);

    if (!HAS_TYPE(symbol, target)) {
      rha_error("Cannot assign to \"%o\"\n", target);
    }

    // now,
    // target: should be a symbol describing the name,
    // scope: should point to the object in which to assign target,
    // lhs: contains the call expression

    //print("assigning %o to %o in %o\n", rhs, target, env);

    // if the object exists, use rha_calldef, else,
    // define a new function
    scope = eval(env, scope);
    object_t o = rha_lookup(scope, target);
    if(!o || !rha_calldefable(o)) {
      int tlen = tuple_length(lhs);
      function_tr fn = function_new(lhs, tlen-1, rhs, env);
      return object_assign(scope, target, fn);
    }
    else {
      // call calldef slot. In this case, we evaluate the "argument variables"
      // For example, if t is a tuple and you have t(i), you would want to have
      // it evaluated, right?
      lhs = tuple_shiftfirst(lhs);
      for(int i = 0; i < tuple_length(lhs); i++)
	tuple_set(lhs, i, eval(env, tuple_get(lhs, i)));
      return rha_calldef(o, lhs, eval(env, rhs));
    }
  }
  else {
    rha_error("Can't assign to left-hand-side.\n");
    return 0;
  }
}


bool iscallof(symbol_tr s, object_t t)
{
  return HAS_TYPE(tuple, t) 
    && (tuple_length(t) > 0)
    && HAS_TYPE(symbol, tuple_get(t, 0)) 
    && symbol_equal_symbol(s, tuple_get(t, 0));
}


/*************************************************************
 *
 * Eval Hooks
 *
 ************************************************************/

object_t eval_lookupfailed_hook(object_t env, object_t symbol)
{
  object_t eobj = rha_lookup(root, eval_sym);
  if(eobj) {
    object_t lookup_hook 
      = rha_lookup(eobj, symbol_new("lookupfailed_hook"));
    if (lookup_hook)
      return object_callslot(eobj, "lookupfailed_hook", 2, env, symbol);
    else
      return 0;
  }
  return 0;
}

object_t eval_fctcall_hook(object_t env, object_t tuple)
{
  static bool in_eval_fctcall_hook = false;

  if(!in_eval_fctcall_hook) {
    object_t eobj = rha_lookup(root, eval_sym);
    if(eobj) {
      object_t fctcall_hook = rha_lookup(eobj, symbol_new("fctcall_hook"));
      if(fctcall_hook) {
	in_eval_fctcall_hook = true;
	object_t retval = object_callslot(eobj, "fctcall_hook", 2, env, tuple);
	in_eval_fctcall_hook = false;
	return retval;
      }
    }
  }
  return tuple;
}

/*************************************************************
 *
 * Frames for 'return', 'break'
 *
 ************************************************************/

// Frames can be opened by functions, loops.  The keyword
// 'return' finishes the most recent function and returns its
// argument, 'break' the most recent loop

jmp_buf frame_stack[FRAME_MAX_NESTING];
object_t frame_value[FRAME_MAX_NESTING];
int frame_type[FRAME_MAX_NESTING];
int frame_tos = -1;

// Note, that these stacks must be only changed via the macros defined
// in eval.h.


