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
#include "rha_types.h"

// stuff for the perl magic
#define rhabarber

/* the rhabarber object */
struct rha_object {
  enum ptypes ptype;            // primtype (ptype): internal type 
                          // that describes the content
  struct symtable *table; // symbol table of slots
  union {
    int i;
    float f;
    double d;
    void *p;              // possible raw content
  } raw;
};

/*
 * Functions
 */
extern       void       object_init(object_t root, object_t module);

extern _rha_ object_t   new();
extern       object_t   new_t(int_t pt, object_t proto);
extern _rha_ address_t  addr(object_t o);
extern _rha_ object_t   clone(object_t parent);
extern _rha_ int_t      ptype(object_t); // primtype
extern       void       setptype(object_t, int_t);

extern _rha_ void       print_fn(object_t);
extern _rha_ string_t   to_string(object_t);

extern _rha_ void       ls(object_t);

extern       object_t   wrap_int(int ptype, object_t proto, int i);
extern       object_t   wrap_float(int ptype, object_t proto, float f);
extern       object_t   wrap_double(int ptype, object_t proto, double d);
extern       object_t   wrap_ptr(int ptype, object_t proto, void *p);
extern       int        unwrap_int(int ptype, object_t o);
extern       float      unwrap_float(int ptype, object_t o);
extern       double     unwrap_double(int ptype, object_t o);
extern       void      *unwrap_ptr(int ptype, object_t o);

// 'lookup'
//     z          -> lookup(local, \z);
//     a.x        -> lookup(a, \x);
extern _rha_ object_t   lookup(object_t env, symbol_t s);

// 'assign'
//     x = 17;    ->  assign(local_sym, \x, 17);
//     a.x = 42;  ->  assign(a, \x, 42);
extern _rha_ object_t   assign(object_t obj, symbol_t s, object_t newobj);
extern _rha_ void       rmslot(object_t, symbol_t s);

// will be called by prule 'include'
extern _rha_ void       include(object_t dest, object_t source);
// will be called by prule 'subscribe'
extern _rha_ void       subscribe(object_t dest, object_t interface);

//extern object_t   ls(object_t o);
//extern object_t   lsall(object_t o);

#endif
