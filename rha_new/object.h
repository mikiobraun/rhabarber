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
#include <stdarg.h>
#include "rha_types.h"

// stuff for the perl magic
#define rhabarber


/*
 * Functions
 */
extern       void       object_init(any_t root, any_t module);

extern _rha_ any_t   new(void);
extern       any_t   create_pt(int pt);
extern       any_t   new_pt(int pt);
extern _ignore_ any_t   create_builtin(builtin_t code, bool varargs, int narg, ...);
extern _ignore_ any_t   vcreate_builtin(builtin_t code, bool varargs, int narg, va_list);
extern _ignore_ any_t   create_ccode(ccode_t code, enum ptypes returntype, bool varargs, int narg, ...);
extern _ignore_ any_t vcreate_ccode(ccode_t code, enum ptypes returntype, bool varargs, int narg, va_list args);

extern       any_t   copy_pt(any_t o);
extern _rha_ address_t  addr(any_t o);
extern _rha_ any_t   clone(any_t parent);
extern _rha_ int      ptype(any_t);      // primtype
extern _rha_ string_t   ptypename(any_t o);
extern       void       setptype(any_t, int);

extern _rha_ void       print_fn(int narg, ...);
extern _rha_ void       vprint_fn(tuple_t args);
extern _rha_ string_t   to_string(any_t);
extern       string_t   to_string_only_in_c(any_t o);
extern _rha_ string_t   builtin_to_string(builtin_t);
extern _rha_ string_t   ccode_to_string(ccode_t);
extern _rha_ string_t   address_to_string(address_t);

extern _rha_ void       ls(any_t);

extern _rha_ bool     equalequal_fn(any_t a, any_t b);
extern _rha_ bool     notequal_fn(any_t a, any_t b);
extern _rha_ any_t   inc(any_t);
extern _rha_ any_t   dec(any_t);
extern _rha_ any_t   inc_copy(any_t);
extern _rha_ any_t   dec_copy(any_t);

extern _ignore_ any_t   wrap_int(int ptype, int i);
extern _ignore_ any_t   wrap_double(int ptype, double d);
extern _ignore_ any_t   wrap_builtin(int ptype, builtin_t b);
extern _ignore_ any_t   wrap_ccode(int ptype, ccode_t c);
extern _ignore_ any_t   wrap_ptr(int ptype, void *p);
extern _ignore_ int        unwrap_int(int ptype, any_t o);
extern _ignore_ double     unwrap_double(int ptype, any_t o);
extern _ignore_ builtin_t  unwrap_builtin(int ptype, any_t o);
extern _ignore_ ccode_t  unwrap_ccode(int ptype, any_t o);
extern _ignore_ void      *unwrap_ptr(int ptype, any_t o);


//        z          -> lookup(local, \z);
//        a.x        -> lookup(a, \x);
extern _rha_ any_t   lookup(any_t env, symbol_t s);
extern _rha_ any_t   lookup_local(any_t l, symbol_t s);
extern _rha_ bool    isparent(any_t p, any_t o);
extern _rha_ bool    check(any_t t, any_t o);
extern _rha_ bool    pcheck(any_t t, any_t o);
extern _rha_ bool    is_void(any_t o);
extern _rha_ any_t   location(any_t l, symbol_t s);

//       x = 17;    ->  assign(local_sym, \x, 17);
//       a.x = 42;  ->  assign(a, \x, 42);
extern _rha_ any_t   assign(any_t obj, symbol_t s, any_t newobj);

//       [x, y] = [17, 42];  ->  assign_many(local_sym, lhs, rhs);
extern _rha_ any_t   assign_many(any_t obj, tuple_t t, any_t newobj);

extern _rha_ any_t   assign_fn(any_t obj, any_t a, any_t newobj);

extern _rha_ any_t   extend(any_t this, symbol_t s, tuple_t args, 
			       any_t env, any_t value);


// 'has'
extern _rha_ bool     has(any_t obj, symbol_t s);

extern _rha_ void       rm(any_t, symbol_t s);

// will be called by prule 'include'
extern _rha_ void       include(any_t dest, any_t source);
// will be called by prule 'subscribe'
extern _rha_ void       subscribe(any_t dest, any_t interface);

//extern any_t   ls(any_t o);
//extern any_t   lsall(any_t o);

#endif
