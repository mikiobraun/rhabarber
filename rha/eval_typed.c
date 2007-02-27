#include "eval.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "debug.h"
#include "builtin_tr.h"
#include "function_tr.h"
#include "core.h"
#include "object.h"


// forward declarations
object_t eval(object_t this, object_t env, object_t expr);
object_t assign(object_t this, object_t env, object_t lexpr, object_t rexpr);
object_t loc(object_t this, object_t env, object_t expr);
bool assignable(object_t lexpr);
bool locable(object_t lexpr);

/* ----------------------------------------------------------
   typing
     bool is_type(object_t expr);
     symbol_tr arg_checkandgetsymbol(object_t arg);
     object_t arg_getdefault(object_t arg);
     object_t arg_gettype(object_t arg);
     bool arg_checktype(object_t expr, object_t type);
*/

bool is_keyword(symbol_tr expr)
{
  // this can be made much faster by lessthan all builtin ids
  CHECK_TYPE(symbol, expr);
  if (symbol_equal_symbol(expr, colon_sym)) return true;
  if (symbol_equal_symbol(expr, do_sym)) return true;
  if (symbol_equal_symbol(expr, dot_sym)) return true;
  if (symbol_equal_symbol(expr, plus_sym)) return true;
  if (symbol_equal_symbol(expr, minus_sym)) return true;
  if (symbol_equal_symbol(expr, times_sym)) return true;
  if (symbol_equal_symbol(expr, divide_sym)) return true;
  if (symbol_equal_symbol(expr, less_sym)) return true;
  if (symbol_equal_symbol(expr, greater_sym)) return true;
  if (symbol_equal_symbol(expr, equal_sym)) return true;
  if (symbol_equal_symbol(expr, maps_sym)) return true;
  if (symbol_equal_symbol(expr, assign_sym)) return true;
  if (symbol_equal_symbol(expr, fn_sym)) return true;
  if (symbol_equal_symbol(expr, if_sym)) return true;
  if (symbol_equal_symbol(expr, parent_sym)) return true;
  if (symbol_equal_symbol(expr, return_sym)) return true;
  if (symbol_equal_symbol(expr, that_sym)) return true;
  if (symbol_equal_symbol(expr, this_sym)) return true;
  if (symbol_equal_symbol(expr, void_sym)) return true;
  return false;
}

bool is_type(object_t expr) {
  if (HAS_TYPE(tuple, expr)) {
    // types are built by "op*", ":"
    if (tuple_length(expr) != 3) return false;
    object_t t0 = tuple_get(expr, 0);
    if (HAS_TYPE(symbol, t0)) {
      if (symbol_equal_symbol(t0, times_sym)) 
	return is_type(tuple_get(expr, 1)) && is_type(tuple_get(expr,2));
      if (symbol_equal_symbol(t0, colon_sym))
	return is_type(tuple_get(expr, 1)) && is_type(tuple_get(expr,2));
    }
    return false;
  }
  if (HAS_TYPE(symbol, expr)) {
    // keywords are not types
    if (!is_keyword(expr)) return true;
  }
  return false;
}

bool arg_check(object_t arg)
{
  if (HAS_TYPE(symbol, arg)) {
    // case 1: neither default nor type
    return true;
  }
  if (HAS_TYPE(tuple, arg)) {
    if (tuple_length(arg) != 3) return 0;
    object_t t0 = tuple_get(arg, 0);
    if (!HAS_TYPE(symbol, t0)) return 0;
    if (symbol_equal_symbol(t0, assign_sym)) {
      object_t t1 = tuple_get(arg, 1);
      if (!HAS_TYPE(symbol, t1)) return 0;
      // case 2: default value w/o type
      return true;
    } 
    if (symbol_equal_symbol(t0, colon_sym)) {
      object_t t2 = tuple_get(arg, 2);  // the type
      if (!is_type(t2)) return 0;       // check that t2 is a type
      object_t t1 = tuple_get(arg, 1);
      if (HAS_TYPE(symbol, t1)) {
	// case 3: typed but no default
	return true;
      }
      if (HAS_TYPE(tuple, t1)) {
	if (tuple_length(t1)!=3) return 0;
	object_t t10 = tuple_get(t1, 0);
	if (!HAS_TYPE(symbol, t10)) return 0;
	if (!symbol_equal_symbol(t10, assign_sym)) return 0;
	object_t t11 = tuple_get(t1, 1);
	if (!HAS_TYPE(symbol, t11)) return 0;
	// case 4: typed with default
	return true;
      }
    }
  }
  return false;  // something went wrong
}

symbol_tr arg_getsymbol(object_t arg)
{
  if (HAS_TYPE(symbol, arg)) {
    // case 1: neither default nor type
    return arg;
  }
  if (HAS_TYPE(tuple, arg)) {
    assert(tuple_length(arg) != 3);
    object_t t0 = tuple_get(arg, 0);
    CHECK_TYPE(symbol, t0);
    if (symbol_equal_symbol(t0, assign_sym)) {
      object_t t1 = tuple_get(arg, 1);
      CHECK_TYPE(symbol, t1);
      // case 2: default value w/o type
      return t1;
    } 
    if (symbol_equal_symbol(t0, colon_sym)) {
      object_t t1 = tuple_get(arg, 1);
      if (HAS_TYPE(symbol, t1)) {
	// case 3: typed but no default
	return t1;
      }
      if (HAS_TYPE(tuple, t1)) {
	assert(tuple_length(t1)!=3);
	CHECK_TYPE(symbol, tuple_get(t1, 0));
	assert(symbol_equal_symbol(t10, assign_sym));
	object_t t11 = tuple_get(t1, 1);
	CHECK_TYPE(symbol, t11);
	// case 4: typed with default
	return t11;
      }
    }
  }
  return 0;  // something went wrong
}

object_t arg_getdefault(object_t arg)
{
  if (HAS_TYPE(tuple, arg)) {
    if (tuple_length(arg) != 3) return 0;
    object_t t0 = tuple_get(arg, 0);
    if (!HAS_TYPE(symbol, t0)) return 0;
    if (symbol_equal_symbol(t0, assign_sym)) {
      object_t t2 = tuple_get(arg, 2);
      // case 2: default value w/o type
      return t2;
    } 
    if (symbol_equal_symbol(t0, colon_sym)) {
      object_t t1 = tuple_get(arg, 1);
      if (HAS_TYPE(tuple, t1)) {
	if (tuple_length(t1)!=3) return 0;
	object_t t10 = tuple_get(t1, 0);
	if (!HAS_TYPE(symbol, t10)) return 0;
	if (!symbol_equal_symbol(t10, assign_sym)) return 0;
	object_t t12 = tuple_get(t1, 1);
	// case 4: typed with default
	return t12;
      }
    }
  }
  return 0;  // no default
}

object_t arg_gettype(object_t arg)
{
  if (HAS_TYPE(tuple, arg)) {
    if (tuple_length(arg) != 3) return 0;
    object_t t0 = tuple_get(arg, 0);
    if (!HAS_TYPE(symbol, t0)) return 0;
    if (symbol_equal_symbol(t0, colon_sym)) {
      object_t t2 = tuple_get(arg, 2);
      assert(is_type(t2));
      return t2;
    }
  }
  return 0;  // something went wrong
}


bool arg_checktype(object_t expr, object_t type)
{
  return true;
}


/*------------------------------------------------------------
  equal expressions
    eval_equal    evaluates the equal sign    
*/

object_t eval_equal(object_t this, object_t env, tuple_tr t) 
{
  if (tuple_length(t) == 3) {
    return assign(this, env, tuple_get(t, 1), tuple_get(t, 2));
  }
  return 0;
}

/*------------------------------------------------------------
  function expressions
    create_fn  creates a new function
    eval_fn    evaluates an function expression without calling
    call_fn    calls function
    assign_fn  assigns new code
    loc_fn     looks up or creates a function
*/

object_t create_fn(tuple_tr t, int numargs, object_t code, object_t env)
{
  function_tr f = function_new(t, numargs, code, env);
  printdebug(" %p.lex -> %p\n", (void *) f, (void *) env);

  // add local variables
  for(int i = 0; i < numargs; i++) {
    object_t arg =  function_arg(f, i);
    if (!arg_check(arg)) return 0;
    symbol_tr s = arg_getsymbol(arg);
    assert(s);  // should exist because we called arg_check
    object_assign(f, s, 0);  // assign zeros to force values during invokations
  }
  return f;
}


object_t eval_fn(object_t this, object_t env, tuple_tr t)
{
  int tlen = tuple_length(t);
  if (tlen > 1) {
    return create_fn(t, tlen-2, tuple_get(t, tlen-1), this);
  }
  return 0;
}


object_t call_fn(object_t this, object_t env, tuple_tr t)
{
  if (tuple_length(t) > 0) {
    object_t f = eval(this, env, tuple_get(t, 0));
    if (!f) return 0;
    if (HAS_TYPE(builtin, f)) return builtin_call(f, this, env, t);
    if (HAS_TYPE(function, f)) return function_call(f, this, env, t);
  }
  return 0;
}


object_t assign_fn(object_t this, object_t env, tuple_tr t, object_t rexpr)
{
  // here t does not contain any code, instead rexpr is the code
  int tlen = tuple_length(t);
  if (tlen > 0) {
    object_t t0 = tuple_get(t, 0);
    if (HAS_TYPE(symbol, t0)) {
      symbol_tr s = t0;
      
      // create a new function with code rexpr
      object_t par = create_fn(t, tlen-1, rexpr, env);
      if (!par) return 0;
      return object_assign(this, s, par);
    }
  }
  return 0;
}

object_t loc_fn(object_t this, object_t env, tuple_tr t) 
{
  // here t does not contain any code
  int tlen = tuple_length(t);
  if (tlen > 0) {
    object_t t0 = tuple_get(t, 0);
    if (HAS_TYPE(symbol, t0)) {
      symbol_tr s = t0;

      // try to look up the function
      object_t par = object_lookup(this, env, s);
      
      // did we find s?
      if (par) {
	// check that par is function
	if (!HAS_TYPE(function, par)) return 0;
	// check number of arguments
	function_tr f = par;
	if (function_numargs(f) != tlen-1) return 0;
	// ok, function found
	return par;
      }
      
      // create a new function without code
      par = create_fn(t, tlen-1, 0, env);
      if (!par) return 0;
      return object_assign(this, s, par);
    }
  }
  return 0;
}

/*------------------------------------------------------------
  symbol
    loc_symbol    looks up or creates a symbol
*/

object_t loc_symbol(object_t this, object_t env, symbol_tr s)
{
  // lookup symbol s
  if (symbol_equal_symbol(s, this_sym)) return this;
  if (symbol_equal_symbol(s, that_sym)) return object_that(this);
  if (symbol_equal_symbol(s, parent_sym)) return object_parent(this);
  object_t obj = object_lookup(this, env, s);
  if (!obj) {
    // create the necessary object
    obj = empty_new();
    object_assign(this, s, obj); 
  }
  return obj;   
}

/*------------------------------------------------------------
  dot expressions
    eval_dot    evaluates the dot
    assign_dot  assigns a left expr to a dotted right expr
    loc_dot     looks up or creates a dotted left expr
*/

object_t eval_dot(object_t this, object_t env, tuple_tr t) 
{
  if (tuple_length(t) == 3) {
    object_t t1 = eval(this, env, tuple_get(t, 1));
    if (t1) { 
      object_t t2 = tuple_get(t, 2);
      if (locable(t2)) {
	if (HAS_TYPE(tuple, t2)) {
	  // t2 is function
	  return call_fn(t1, env, t2);
	}
	else {
	  // t2 is symbol
	  CHECK_TYPE(symbol, t2);
	  return object_lookup(t1, env, t2);
	}
      }
    }
  }
  return 0;
}

object_t assign_dot(object_t this, object_t env, tuple_tr t, object_t rexpr) 
{
  // "assign_dot" can only be called once per call of "assign"
  if (tuple_length(t) == 3) {
    object_t t1 = loc(this, env, tuple_get(t, 1));
    if (t1) {
      object_t t2 = tuple_get(t, 2);
      if (assignable(t2)) {
	object_t obj;
	if (HAS_TYPE(tuple, t2)) {
	  // t2 is function
	  obj = assign_fn(t1, env, t2, rexpr);
	}
	else {
	  // t2 is symbol
	  CHECK_TYPE(symbol, t2);
	  obj = rexpr;
	  object_assign(t1, t2, rexpr);
	}
	if (!obj) return 0;
	object_t that = object_that(obj);
	if (that) assert(that==t1);
	else object_setthat(obj, t1);
	return obj;
      }
    }
  }
  return 0;
}

object_t loc_dot(object_t this, object_t env, tuple_tr t) 
{
  // "loc_dot" can be only called once per call to "loc"
  if (tuple_length(t) == 3) {
    object_t t1 = loc(this, env, tuple_get(t, 1));
    if (t1) {
      object_t t2 = tuple_get(t, 2);
      if (locable(t2)) {
	object_t obj;
	if (HAS_TYPE(tuple, t2)) {
	  // t2 is function
	  obj = loc_fn(t1, env, t2);
	}
	else {
	  // t2 is symbol
	  CHECK_TYPE(symbol, t2);
	  obj = loc_symbol(t1, env, t2);
	}
	if (!obj) return 0;
	object_t that = object_that(obj);
	if (that) assert(that==t1);
	else object_setthat(obj, t1);
	return obj;
      }
    }
  }
  return 0;
}



/*------------------------------------------------------------
  the general functions
*/

object_t eval(object_t this, object_t env, object_t expr)
     // this   is the dynamic closure to look up expr, e.g. in a.b(c) 
     // leads to the following calls
     //        eval(env, env, "a.b(c)");
     //        eval(a, env, "b(c)");
     //        eval(env, env, "c");     // in function_call()
     //        "this" is needed for the dot notation

     // env    is the surrounding environment, the lexical closure to
     //        evaluated free variables in inputs for function calls; 
     //        used for function_call()

     // expr   is the expression to be evaluated
{
  printdebug("eval(this=%p, env=%p, expr=%p %o)\n", (void *) this, (void *) env, (void *) expr, expr);

  if (HAS_TYPE(tuple, expr)) {
    tuple_tr t = expr;
    object_t t0 = tuple_get(t, 0);
    if (HAS_TYPE(symbol, t0)) {
      // catch keywords
      if (symbol_equal_symbol(dot_sym, t0)) return eval_dot(this, env, t);
      if (symbol_equal_symbol(assign_sym, t0)) return eval_equal(this, env, t);
      if (symbol_equal_symbol(fn_sym, t0)) return eval_fn(this, env, t);
    }
    // otherwise call function
    return call_fn(this, env, t);
  }
  else if (HAS_TYPE(symbol, expr)) {
    // catch keywords
    if (symbol_equal_symbol(this_sym, expr)) return this;
    if (symbol_equal_symbol(that_sym, expr)) return object_that(this);
    if (symbol_equal_symbol(parent_sym, expr)) return object_parent(this);
    // otherwise do dynamic/lexical lookup 
    return object_lookup(this, env, expr);
  }
  else return expr;
}


object_t assign(object_t this, object_t env, object_t lexpr, object_t rexpr)
{
  printdebug("assign(this=%p, env=%p, lexpr=%p %o, rexpr=%p %o)\n", 
	     this, env, lexpr, lexpr, rexpr, rexpr);

  // this should parallel eval.c

  if (HAS_TYPE(tuple, lexpr)) {
    tuple_tr t = lexpr;
    object_t t0 = tuple_get(t, 0);
    if (HAS_TYPE(symbol, t0)) {
      // assign rexpr to t0
      if (symbol_equal_symbol(t0, dot_sym)) return assign_dot(this, env, t, rexpr);
      if (symbol_equal_symbol(t0, assign_sym)) return 0;   // not allowed
      if (symbol_equal_symbol(t0, fn_sym)) return 0;  // not allowed
      return assign_fn(this, env, t, rexpr);
    }
    // note that different to eval() here the first entry of t must be a symbol
  }
  else if (HAS_TYPE(symbol, lexpr)) {
    // find the right place
    symbol_tr s = lexpr;

    if (symbol_equal_symbol(s, this_sym)) return 0; // not allowed

    // evaluate right side
    object_t robj = eval(this, env, rexpr);
    if (!robj) return 0;

    // assign robj to the symbol lexpr
    if (symbol_equal_symbol(s, that_sym)) return object_setthat(this, robj);
    if (symbol_equal_symbol(s, parent_sym)) return object_setparent(this, robj);
    return object_replace(this, env, s, robj);
  }
  // note that expr should contain only symbols, functions and dot expressions
  return 0;
}

object_t loc(object_t this, object_t env, object_t expr)
{
  printdebug(stderr, "loc(this=%p, env=%p, expr=%p %o)\n", 
	     this, env, expr, expr);
  
  // expr must consists of symbols, functions
  // this should parallel eval.c
  
  if (HAS_TYPE(tuple, expr)) {
    tuple_tr t = expr;
    object_t t0 = tuple_get(t, 0);
    if (HAS_TYPE(symbol, t0)) {
      // call loc for symbol t0
      if (symbol_equal_symbol(t0, dot_sym)) return loc_dot(this, env, t);
      if (symbol_equal_symbol(t0, assign_sym)) return 0;   // not allowed
      if (symbol_equal_symbol(t0, fn_sym)) return 0;  // not allowed
      return loc_fn(this, env, t);
    }
    // note that different to eval() here the first entry of t must be a symbol
  }
  else if (HAS_TYPE(symbol, expr)) return loc_symbol(this, env, expr);
  // note that expr should contain only symbols, functions and dot expressions
  return 0;
}


bool assignable(object_t expr)
{
  // this should parallel eval.c

  if (HAS_TYPE(tuple, expr)) {
    // tuple is assignable if the function is not one of ".", "=", "fn"
    object_t t0 = tuple_get(expr, 0);
    if (HAS_TYPE(symbol, t0)) {
      if (symbol_equal_symbol(t0, dot_sym)) return false;   // not allowed
      if (symbol_equal_symbol(t0, assign_sym)) return false;   // not allowed
      if (symbol_equal_symbol(t0, fn_sym)) return false;  // not allowed
      return true;
    }
  }
  else if (HAS_TYPE(symbol, expr)) {
    // anything but "this" is assignable
    if (symbol_equal_symbol(expr, this_sym)) return false;
    return true;
  }
  return false;
}


bool locable(object_t expr)
{
  // this should parallel eval.c

  if (HAS_TYPE(tuple, expr)) {
    // anything but the functions ".", "=", and "fn"
    object_t t0 = tuple_get(expr, 0);
    if (HAS_TYPE(symbol, t0)) {
      if (symbol_equal_symbol(t0, dot_sym)) return false;   // not allowed
      if (symbol_equal_symbol(t0, assign_sym)) return false;   // not allowed
      if (symbol_equal_symbol(t0, fn_sym)) return false;  // not allowed
      return true;
    }
  }
  else if (HAS_TYPE(symbol, expr)) {
    // "this", "that", "parent" and other symbols are locable
    return true;
  }
  return false;
}

