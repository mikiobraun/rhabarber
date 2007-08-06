
#include <stdarg.h>
#include "object.h"


object_t eval(object_t env, object_t expr)
{
  ENTER;
  //print("eval(env = %p, expr = %o)\n", env, expr);

  switch (primtype(expr)) {
  case SYM_T: 
    // symbols
    RETURN( lookup(env, expr) );
  case TUPLE_T: 
    // function call
    RETURN( call_fun(env, expr) );
  default:
    // literal
    RETURN( expr );
  }
}


object_t call_fun(object_t env, tuple_t expr)
{
  int tlen = tuple_length(expr);
  assert(tlen>0);  // otherwise repair 'rhaparser.y'
  object_t f = tuple_get(expr, 0);
  // deal with 'quote'
  if ((primtype(f)==SYM_T) && symbol_equal_symbol(quote_sym, f)) {
    assert(tlen==2);  // otherwise repair 'rhaparser.y'
    return tuple_get(expr, 1);
  }
  // otherwise a usual function
  for (int i=0; i<tlen; i++)
    // does the next line?
    // or do we have to allocate a new tuple
    tuple_set(expr, i, eval(env, tuple_get(expr, i)));
  if (primtype(tuple_get(expr, 0))==FN_T)
    // the function is implemented in C
    return call_C_fun(tlen, expr);
  else
    // the function is pure rhabarber
    return call_rha_fun(tlen, expr);
}


void *get_and_check(object_t o, int_t ptype)
{
  if (primtype(o) != ptype)
    rha_error("argument primtype missmatch\n");
  return raw(o);
}


object_t call_C_fun(int tlen, tuple_t t) 
{
  // more arguments needed?  increase here!
  int maxnargs = 5;
  void *args[maxnargs];
  narg = tlen-1;
  assert(narg<=maxnargs);
  fn_t f = tuple_get(t, 0);
  for (int i=0; i<narg; i++)
    args[i] = get_n_check(tuple_get(t, i+1), f->argtypes[0]);

  // finally call 'f'
  // note that f->code must be properly casted before calling.
  // If someone knows a less ugly way without restricting the total
  // number of arg please let Mikio and Stefan know!
  switch (narg) {
  case 0: 
    return ((void *(*)()) f->code)();
  case 1: 
    return ((void *(*)(void *)) f->code)(*args[0]);
  case 2: 
    return ((void *(*)(void *, void *)) f->code)(*args[0], *args[1]);
  case 3: 
    return ((void *(*)(void *, void *, void *)) f->code)(*args[0], *args[1], *args[2]);
  case 4: 
    return ((void *(*)(void *, void *, void *, void *)) f->code)(*args[0], *args[1], *args[2], *args[3]);
  case 5: 
    return ((void *(*)(void *, void *, void *, void *, void *)) f->code)(*args[0], *args[1], *args[2], *args[3], *args[4]);
  }
}


object_t call_rha_fun(int narg, object_t expr)
{
  // not yet
  // this should be done via some slot access
}
