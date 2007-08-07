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

/* the rhabarber object */
struct rha_object {
  int_t ptype;            // primtype (ptype): internal type 
                          // that describes the content
  struct symtable *table; // symbol table of slots
  void *raw;              // possible raw content
};

/*
 * Functions
 */
extern _rha_ object_t   new();
extern        object_t   new_t();
extern _rha_ object_t   clone(object_t parent);
extern _rha_ int_t      ptype(object_t); // primtype
extern        void       setptype(object_t, int_t);
extern        void      *raw(object_t o);
extern        void       setraw(object_t, void *);

// 'lookup'
//     z          -> lookup(local, \z);
//     a.x        -> lookup(a, \x);
extern _rha_ object_t   lookup(object_t env, symbol_t s);

// 'assign'
//     x = 17;    ->  assign(local_sym, \x, 17);
//     a.x = 42;  ->  assign(a, \x, 42);
extern _rha_ object_t   assign(object_t obj, symbol_t s, object_t newobj);
extern _rha_ void_t     rmslot(object_t, symbol_t s);
extern _rha_ void_t     print(object_t);

// will be called by prule 'include'
extern _rha_ void_t     include(object_t dest, object_t source);
// will be called by prule 'subscribe'
extern _rha_ void_t     subscribe(object_t dest, object_t interface);

extern _rha_ object_t   ls(object_t o);
extern _rha_ object_t   lsall(object_t o);

#endif
