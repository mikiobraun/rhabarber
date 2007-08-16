#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>
#include "eval.h"
#include "object.h"
#include "rha_types.h"
#include "messages.h"
#include "utils.h"
#include "list_fn.h"
#include "tuple_fn.h"
#include "symbol_fn.h"


void eval_init(object_t root, object_t module)
{
  // the only two function we need to pull by hand (and not in prelude.rha)
  assign(root, eval_sym, lookup(module, eval_sym));
}



// forward declarations
object_t eval_sequence(object_t env, list_t source);
object_t eval_args_and_call_fun(object_t env, tuple_t expr);
object_t call_fun(object_t env, int_t tlen, tuple_t expr);
void *call_C_fun(int tlen, tuple_t t);
object_t call_rha_fun(object_t this, int narg, tuple_t expr);

// some code for bug-tracking
object_t eval_currentlocation = 0;

#define ENTER     object_t savedloc = eval_currentlocation; eval_currentlocation = expr
#define RETURN(x) do { object_t retval = (x); \
                           eval_currentlocation = savedloc; \
                           return retval; } while(0)

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

#define IGNORE
#ifndef IGNORE
object_t eval(object_t env, object_t expr)
{
  ENTER;
  print("eval(env = %p, expr = %o)\n", env, expr);

  object_t value;
  switch (ptype(expr)) {
  case SYMBOL_T:
    // symbols
    value = lookup(env, UNWRAP_SYMBOL(expr));
    if (!value) rha_error("lookup of symbol '%o' failed\n", expr);
    RETURN( value ); 
  case TUPLE_T: 
    // function call
    assert(UNWRAP_PTR(TUPLE_T, expr));
    RETURN( eval_args_and_call_fun(env, UNWRAP_PTR(TUPLE_T, expr)) );
  case LIST_T:
    // sequence of expressions, e.g. { x=17; y=42 }
    assert(UNWRAP_PTR(LIST_T, expr));
    RETURN( eval_sequence(env, UNWRAP_PTR(LIST_T, expr)) );
  default:
    // literal
    RETURN( expr );
  }
}
#endif

object_t eval(object_t env, object_t expr)
{
  print("eval(env=%p, expr=%o)\n", env, expr);

  object_t value;
  switch (ptype(expr)) {
  case SYMBOL_T:
    // symbols
    value = lookup(env, UNWRAP_SYMBOL(expr));
    if (!value) rha_error("lookup of symbol '%o' failed\n", expr);
    return value;
  case TUPLE_T: 
    // function call
    assert(UNWRAP_PTR(TUPLE_T, expr));
    return eval_args_and_call_fun(env, UNWRAP_PTR(TUPLE_T, expr));
  case LIST_T:
    // sequence of expressions, e.g. { x=17; y=42 }
    assert(UNWRAP_PTR(LIST_T, expr));
    return eval_sequence(env, UNWRAP_PTR(LIST_T, expr));
  default:
    // literal
    return expr;
  }
}


object_t eval_sequence(object_t env, list_t source)
{
  // either the value of the last expression is delivered
  // or the value following 'deliver'
  // eval_squence does not open a new scope
  object_t res = void_obj;
  list_it it;
  begin_frame(BLOCK_FRAME)
    // evaluate all, or stop earlier via 'deliver', 'break', 'return'
    for (list_begin(source, &it); !list_done(&it); list_next(&it))
      res = eval(env, list_get(&it));
  end_frame(res);
  // return the result
  return res;
}


object_t eval_args_and_call_fun(object_t env, tuple_t expr)
{
  int tlen = tuple_len(expr);
  assert(tlen>0);  // otherwise repair 'rhaparser.y'
  object_t f = tuple_get(expr, 0);
  // deal with 'quote'
  if ((ptype(f)==SYMBOL_T) && symbol_equal(quote_sym, UNWRAP_SYMBOL(f))) {
    assert(tlen==2);  // otherwise repair 'rhaparser.y'
    return tuple_get(expr, 1);
  }

  // otherwise a usual function
  tuple_t values = tuple_new(tlen);
  for (int i=0; i<tlen; i++)
    tuple_set(values, i, eval(env, tuple_get(expr, i)));

  // finally call the function
  return call_fun(env, tlen, values);
}

object_t call_fun(object_t env, int_t tlen, tuple_t values)
{
  if (ptype(tuple_get(values, 0))==FUNCTION_T)
    // the function is implemented in C
    return call_C_fun(tlen, values);
  else
    // the function is pure rhabarber
    return call_rha_fun(env, tlen, values);
}


void check_ptypes(object_t o, enum ptypes pt)
{
  // note that OBJECT_T matches anything!
  if ((pt != OBJECT_T) && (ptype(o) != pt))
    rha_error("argument primtype missmatch (expected: %s, got: %s)\n",
	      ptype_names[pt], ptype_names[ptype(o)]);
}


void *call_C_fun(int tlen, tuple_t t) 
{
  int narg = tlen-1;
  // extract the function
  object_t t0 = tuple_get(t, 0);
  assert(t0 && ptype(t0)==FUNCTION_T);
  function_t f = UNWRAP_PTR(FUNCTION_T, t0);
  // is the argument number correct?
  if (f->narg != narg)
    rha_error("wrong number of arguments (expected %d, got %d)\n", f->narg, narg);
  // check the types of the args
  for (int i=0; i<narg; i++)
    check_ptypes(tuple_get(t, i+1), f->argptypes[i]);

  // finally call 'f'
  return (f->code)(t);
}

/* Call a rhabarber function 
 *
 * Checks if the number of argument matches, constructs the local
 * environment of the callee and executes the function
 *
 * callable objects must have slots "argnames", "scope", and "fnbody".
 */
object_t call_rha_fun(object_t this, int tlen, tuple_t expr)
{
  object_t fn = tuple_get(expr, 0);

  // check if number of args is the same
  int nargs = tlen - 1;
  object_t _argnames = lookup(fn, argnames_sym);
  if (!_argnames) {
    rha_error("the object %o is not callable\n", fn);
    return 0;
  }
  tuple_t argnames = UNWRAP_PTR(TUPLE_T, _argnames);

  if (tuple_len(argnames) != nargs) {
    rha_error("Function called with wrong number of arguments");
  }

  // construct the inner scope
  object_t local = new();
  assign(local, local_sym, local);
  assign(local, this_sym, this);
  assign(local, parent_sym, lookup(fn, scope_sym));

  // assign the arguments
  for(int i = 0; i < nargs; i++) {
    symbol_t s = UNWRAP_SYMBOL(tuple_get(argnames, i));
    printf("assigning argument number %d to '%s'\n", i, symbol_name(s));
    assign(local, s, tuple_get(expr, i+1));
  }

  // execute the function body
  object_t fnbody = lookup(fn, fnbody_sym);
  return eval(local, fnbody);
}
