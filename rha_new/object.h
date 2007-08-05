/*
 * object - objects with slots
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include "datatypes.h"

// stuff for the perl magic
#define rhafun

/*
 * The rhabarber object
 */
struct rha_object {
  int_t ptype;            // primtype (ptype): internal type 
                          // that describes the content
  struct symtable *table; // symbol table of slots
  void *raw;              // possible raw content
};

/*
 * Functions
 */
extern rhafun object_t   new();
extern rhafun object_t   clone(object_t parent);
extern rhafun int_t      ptype(object_t); // primtype
extern        void      *raw(object_t o);

extern rhafun object_t   lookup(object_t env, symbol_t s);
// will be accessed via '='
//     x = 17;    ->  assign_fn(local_sym, \x, 17);
//     a.x = 42;  ->  assign_fn(a, \x, 42);
extern rhafun object_t   assign_fn(object_t obj, symbol_t s, object_t newobj);
extern rhafun void_t     rmslot(object_t, symbol_t s);
extern rhafun void_t     print(object_t);

// will be called by prule 'include'
extern rhafun void_t     include_fn(object_t dest, object_t source);
// will be called by prule 'subscribe'
extern rhafun void_t     subscribe_fn(object_t dest, object_t interface);

extern rhafun object_t   ls(object_t o);
extern rhafun object_t   lsall(object_t o);


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
