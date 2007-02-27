#include "tuple_tr.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "eval.h"

#include "object.h"
#include "primtype.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "list_tr.h"

#define TUPLE_IMPLEMENTATION
#ifdef USE_INLINES
#define INLINE inline
#else
#define INLINE
#endif 

#include "tuple_inline.c"


#define CHECK(x) CHECK_TYPE(tuple, x)

primtype_t tuple_type = 0;


void tuple_init(void)
{
  if (!tuple_type) {
    tuple_type = primtype_new("tuple", sizeof(struct tuple_s));
    
    (void) tuple_new(0);
  }
}


tuple_tr tuple_new(int length)
{
  tuple_tr t = object_new(tuple_type);
  tupleobject_init(&t->tuple, length);
  return t;
}


tuple_tr tuple_make_unstable(object_t first, ...)
{
  // count arguments
  va_list ap;
  int numargs = 1;
  va_start(ap, first);
  while(va_arg(ap, object_t)) numargs++;
  va_end(ap);

  // construct tuple
  object_t t = tuple_new(numargs);
  tuple_set(t, 0, first);

  va_start(ap, first);
  for(int i = 1; i < numargs; i++)
    tuple_set(t, i, va_arg(ap, object_t));
  va_end(ap);

  return t;
}


tuple_tr tuple_make(int narg, ...)
{
  va_list ap;
  tuple_tr t = tuple_new(narg);
  va_start(ap, narg);
  for (int i = 0; i < narg; i++)
    tuple_set(t, i, va_arg(ap, object_t));
  va_end(ap);

  return t;
}


tuple_tr tuple_make1(object_t o)
{
  tuple_tr t = tuple_new(1);
  tuple_set(t, 0, o);
  return t;
}


tuple_tr tuple_make2(object_t o1, object_t o2)
{
  tuple_tr t = tuple_new(2);
  tuple_set(t, 0, o1);
  tuple_set(t, 1, o2);
  return t;
}


tuple_tr tuple_make3(object_t o1, object_t o2, object_t o3)
{
  tuple_tr t = tuple_new(3);
  tuple_set(t, 0, o1);
  tuple_set(t, 1, o2);
  tuple_set(t, 2, o3);
  return t;
}

tuple_tr tuple_shiftfirst(tuple_tr t)
{
  CHECK(t);
  int l = tuple_length(t);
  tuple_tr u = tuple_new(l-1);
  for(int i = 1; i < l; i++)
    tuple_set(u, i-1, tuple_get(t, i));
  return u;
}

string_t tuple_to_string(tuple_tr t)
{
  CHECK(t);
  string_t s = gc_strdup("(");
  int tlen = tuple_length(t);
  for (int i=0; i<tlen; i++) {
    if (i) s = string_plus_string(s, ", ");
    s = string_plus_string(s, object_to_string(tuple_get(t, i)));
  }
  s = string_plus_string(s, ")");
  return s;
}


object_t tuple_to_list(tuple_tr t)
{
  CHECK(t);
  int tlen = tuple_length(t);
  list_tr l = list_new();
  for (int i=0; i<tlen; i++) {
    list_append(l, tuple_get(t, i));
  }
  return l;
}


bool tuple_less_tuple(tuple_tr t1, tuple_tr t2)
{
  CHECK(t1);
  CHECK(t2);
  int len1 = tuple_length(t1);
  int len2 = tuple_length(t2);
  for (int i = 0; i<((len1<len2) ? len1 : len2); i++) {
    object_t o1 = tuple_get(t1, i);
    object_t o2 = tuple_get(t2, i);
    if (o1!=o2) return o1<o2;
    // should use this:  currently we don't do it 
    // because overloading wouldn't work
    //if (object_less_object(o1, o2)) return true;
    //if (object_less_object(o2, o1)) return false;
  }
  // shorter tuples are smaller
  return (len1<len2);
}


#undef CHECK
