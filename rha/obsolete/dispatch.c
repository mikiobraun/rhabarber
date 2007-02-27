#include <stdio.h>
#include "symtable.h"
#include "object.h"
#include "list.h"
#include "dispatch.h"
#include "builtin.h"

object_t dispatch(object_t o, symbol_t s, list_t l)
{
  if(!o) return 0;

  object_t f = lookup(o, s);
  
  if(!f) return 0;

  builtin_t b;
  if( (b = SAFECAST(builtin, f)) ) {
    pushfrontoflist(l, o);
    return callbuiltin(b, l);
  }
  else
    return 0;
}


// Convenience functions
object_t dispatch0(object_t o, symbol_t s)
{
  return dispatch(o, s, newlist());
}


object_t dispatch1(object_t o, symbol_t s, object_t in1)
{
  list_t l = newlist();
  pushbackoflist(l, in1);
  return dispatch(o, s, l);
}


object_t dispatch2(object_t o, symbol_t s, object_t in1, object_t in2)
{
  list_t l = newlist();
  pushbackoflist(l, in1);
  pushbackoflist(l, in2);
  return dispatch(o, s, l);
}


object_t dispatch3(object_t o, symbol_t s, 
		   object_t in1, object_t in2, object_t in3)
{
  list_t l = newlist();
  pushbackoflist(l, in1);
  pushbackoflist(l, in2);
  pushbackoflist(l, in3);
  return dispatch(o, s, l);
}


