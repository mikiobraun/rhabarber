#include <messages.h>

#ifndef TUPLE_IMPLEMENTATION
struct tuple_s
{
  RHA_OBJECT;
  tupleobject_t tuple;
};
#endif

#ifndef CHECK
#define CHECK(x) CHECK_TYPE(tuple, x)
#define UNDEF_CHECK
#endif

#if defined(TUPLE_IMPLEMENTATION) || defined(USE_INLINES)
INLINE
int tuple_length(tuple_tr t)
{
  CHECK(t);
  return tupleobject_length(&t->tuple);
}


INLINE
object_t *tuple_array(tuple_tr t)
{
  CHECK(t);
  return tupleobject_array(&t->tuple);
}


INLINE
void tuple_set(tuple_tr t, int i, object_t o)
{
  CHECK(t);
  if( !(i >= 0 && i < tupleobject_length(&t->tuple)) )
    rha_error("tuple access error: index out of bounds\n");
  tupleobject_set(&t->tuple, i, o);
}


INLINE
object_t tuple_get(tuple_tr t, int i)
{
  CHECK(t);
  if( !(i >= 0 && i < tupleobject_length(&t->tuple)) )
    rha_error("tuple access error: index out of bounds\n");
  return tupleobject_get(&t->tuple, i);
}


INLINE
tuple_tr tuple_copy(tuple_tr source)
{
  CHECK(source);
  tuple_tr dest = object_new(tuple_type);
  tupleobject_copy(&dest->tuple, &source->tuple);
  return dest;
}


#endif

#ifdef UNDEF_CHECK
#undef CHECK
#endif
