
#include <stdarg.h>
#include <assert.h>
#include "object.h"
#include "datatypes.h"
#include "messages.h"
#include "tuple_fn.h"
#include "symbol_fn.h"

#define MAXNARGS 5

// CORESYMBOLS
extern symbol_t quote_sym;

// forward declarations
object_t call_fun(object_t env, tuple_t expr);
void *call_C_fun(int tlen, tuple_t t);
object_t call_rha_fun(int narg, tuple_t expr);

// some code for bug-tracking
object_t eval_currentlocation = 0;

#define ENTER     object_t savedloc = eval_currentlocation; eval_currentlocation = expr
#define RETURN(x) do { object_t retval = (x); \
                           eval_currentlocation = savedloc; \
                           return retval; } while(0)

object_t eval(object_t env, object_t expr)
{
  ENTER;
  //print("eval(env = %p, expr = %o)\n", env, expr);

  switch (ptype(expr)) {
  case SYMBOL_T: 
    // symbols
    ASSERT_RAW_NONZERO(expr);
    RETURN( lookup(env, *RAW(symbol_t, expr)) );
  case TUPLE_T: 
    // function call
    ASSERT_RAW_NONZERO(expr);
    RETURN( call_fun(env, *RAW(tuple_t, expr)) );
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
  if ((ptype(f)==SYMBOL_T) && symbol_equal(quote_sym, *RAW(symbol_t, f))) {
    assert(tlen==2);  // otherwise repair 'rhaparser.y'
    return tuple_get(expr, 1);
  }
  // otherwise a usual function
  for (int i=0; i<tlen; i++)
    // does the next line?
    // or do we have to allocate a new tuple
    tuple_set(expr, i, eval(env, tuple_get(expr, i)));
  if (ptype(tuple_get(expr, 0))==FN_T)
    // the function is implemented in C
    return call_C_fun(tlen, expr);
  else
    // the function is pure rhabarber
    return call_rha_fun(tlen, expr);
}


void *get_and_check(object_t o, int_t pt)
{
  if (!o) rha_error("undefined primtype object\n");
  if (ptype(o) != pt)
    rha_error("argument primtype missmatch\n");
  return raw(o);
}


void *call_C_fun(int tlen, tuple_t t) 
{
  // more arguments needed?  increase here!
  void *args[MAXNARGS];
  int narg = tlen-1;
  assert(narg<=MAXNARGS);
  fn_t *f = raw(tuple_get(t, 0));
  for (int i=0; i<narg; i++)
    args[i] = get_and_check(tuple_get(t, i+1), f->argtypes[i]);

  // finally call 'f'
  // note that f->code must be properly casted before calling.
  // If someone knows a less ugly way without restricting the total
  // number of arg please let Mikio and Stefan know!
  switch (narg) {
  case 0: 
    return ((void *(*)()) f->code)();
  case 1: 
    return ((void *(*)(void *)) f->code)(args[0]);
  case 2: 
    return ((void *(*)(void *, void *)) f->code)(args[0], args[1]);
  case 3: 
    return ((void *(*)(void *, void *, void *)) f->code)(args[0], args[1], args[2]);
  case 4: 
    return ((void *(*)(void *, void *, void *, void *)) f->code)(args[0], args[1], args[2], args[3]);
  case 5: 
    return ((void *(*)(void *, void *, void *, void *, void *)) f->code)(args[0], args[1], args[2], args[3], args[4]);
  }

  double x = 8.0;

  void *p2 = * (void**) &x;
}


object_t call_rha_fun(int narg, tuple_t expr)
{
  // not yet
  // this should be done via some slot access
}
