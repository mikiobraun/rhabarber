/*
 *
 * Access functions
 *
 */

#include "debug.h"

#ifndef OBJ
#define OBJ(x) ((struct rha_object *)(x))
#define UNDEF_obj
#endif

#if defined(OBJECT_IMPLEMENTATION) || defined(USE_INLINES) 
INLINE 
primtype_t object_primtype(object_t o)
     // get type of an object
{
  return OBJ(o)->ptype;
}


INLINE 
object_t object_parent(object_t o)
     // get pointer to parent of object
{
  return OBJ(o)->parent;
}


INLINE
object_t object_setparent(object_t o, object_t parent)
{
  printdebug(" %p.parent -> %p\n", (void *) o, (void *) parent);
  return (OBJ(o)->parent = parent);
}


INLINE
struct symtable *object_table(object_t o)
{
  return OBJ(o)->table;
}



#endif // USE_INLINES

#ifdef UNDEF_obj
#undef OBJ
#endif
