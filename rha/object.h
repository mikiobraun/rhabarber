/*
 * object - dynamically typed objects with slots
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include "primtype.h"

// stuff for the perl magic
#define constructor
#define method
#define operator
#define constant

/*
 * Type definitions and forward references
 */
typedef const char *string_t;
typedef struct list_s *list_t;

struct listobject;
struct symbol_s;  // declaration to avoid "symbol_tr.h"

/*
 * The rhabarber object
 */
struct rha_object {
  struct symtable *slots;    // symbol table of slots

  int id;
  void *content;
};

typedef void *object_t;

/*
 * Functions
 */
extern object_t   object_new(primtype_t p);
extern object_t   object_clone(object_t parent);

extern primtype_t object_primtype(object_t);
extern object_t   object_parent(object_t);
extern object_t   object_setparent(object_t o, object_t parent);

extern struct symtable *object_table(object_t o);

extern object_t   object_lookup(object_t env, struct symbol_s *s);
extern object_t   object_lookuplocal(object_t this, struct symbol_s *s);
extern object_t   object_replace(object_t env, struct symbol_s *s, 
				 object_t newobj);
extern object_t   object_assign(object_t this, struct symbol_s *s, 
				object_t newobj);
extern void       object_delete(object_t, struct symbol_s *);

extern object_t   object_callslot(object_t o, string_t slotname, 
				  int narg, ...);
extern void       object_print(object_t);

// the following functions are visible in rhabarber via 'core.c'
extern void       object_snatch(object_t dest, object_t source);
extern void       object_match(object_t dest, object_t interface);

extern string_t   object_to_string(object_t obj);
extern string_t   object_name(object_t obj);

extern list_t     object_slots(object_t o);
extern list_t     object_allslots(object_t o);

extern string_t   object_typename(object_t obj);

/* type checking macros */
#define HAS_TYPE(typename, x) (object_primtype(x) == typename##_type)
#define CHECK_TYPE(typename, x) assert(object_primtype(x) == typename##_type)

/* macro for explicitly casting to an object_t.
 * This is necessary for passing objects through "..." safely
 */
#define _O	(object_t)

// inlines
#ifndef OBJECT_IMPLEMENTATION
#define INLINE extern inline
#include "object_inline.c"
#undef INLINE
#endif


#endif
