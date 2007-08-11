#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>
#include "eval.h"
#include "object.h"
#include "rha_types.h"
#include "messages.h"
#include "tuple_fn.h"
#include "symbol_fn.h"

// forward declarations
object_t call_fun(object_t env, tuple_t expr);
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


object_t eval(object_t env, object_t expr)
{
  ENTER;
  //print("eval(env = %p, expr = %o)\n", env, expr);

  switch (ptype(expr)) {
  case SYMBOL_T: 
    // symbols
    RETURN( lookup(env, UNWRAP_SYMBOL(expr)) );
  case TUPLE_T: 
    // function call
    assert(UNWRAP_PTR(TUPLE_T, expr));
    RETURN( call_fun(env, UNWRAP_PTR(TUPLE_T, expr)) );
  default:
    // literal
    RETURN( expr );
  }
}


object_t call_fun(object_t env, tuple_t expr)
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
  tuple_t args = tuple_new(tlen);
  for (int i=0; i<tlen; i++)
    tuple_set(args, i, eval(env, tuple_get(expr, i)));

  if (ptype(tuple_get(args, 0))==FN_T)
    // the function is implemented in C
    return call_C_fun(tlen, args);
  else
    // the function is pure rhabarber
    return call_rha_fun(env, tlen, args);
}


void check(object_t o, int_t pt)
{
  if (ptype(o) != pt) rha_error("argument primtype missmatch (expected: %d, got: %d)\n", pt, ptype(o));
  //if (!raw(o)) rha_error("undefined primtype object\n");
}


void *call_C_fun(int tlen, tuple_t t) 
{
  int narg = tlen-1;
  // extract the function
  object_t t0 = tuple_get(t, 0);
  assert(t0 && ptype(t0)==FN_T);
  fn_t f = UNWRAP_PTR(FN_T, t0);
  // is the argument number correct?
  if (f->narg != narg)
    rha_error("wrong number of arguments (expected %d, got %d)\n", f->narg, narg);
  // check the types of the args
  for (int i=0; i<narg; i++)
    check(tuple_get(t, i+1), f->argtypes[i]);

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
    printf("the object is not callable\n");
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
