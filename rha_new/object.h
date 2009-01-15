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

extern  any_t   new(void);
extern       any_t   create_pt(int pt);
extern       any_t   new_pt(int pt);
extern _ignore_ any_t   create_builtin(builtin_t code, bool varargs, int narg, ...);
extern _ignore_ any_t   vcreate_builtin(builtin_t code, bool varargs, int narg, va_list);
extern _ignore_ any_t   create_ccode(ccode_t c, bool varargs, int narg, ...);
extern _ignore_ any_t vcreate_ccode(ccode_t c, bool varargs, int narg, va_list args);

extern       any_t   copy_pt(any_t o);
extern  address_t  addr(any_t o);
extern  any_t   clone(any_t parent);
extern  int      ptype(any_t);      // primtype
extern  string_t   ptypename(any_t o);
extern       void       setptype(any_t, int);

extern  void       print_fn(int narg, ...);
extern  void       vprint_fn(tuple_t args);
extern  string_t   to_string(any_t);
extern       string_t   to_string_only_in_c(any_t o);
extern  string_t   builtin_to_string(builtin_t);
extern  string_t   ccode_to_string(ccode_t);
extern  string_t   address_to_string(address_t);

extern  void       ls(any_t);

extern  bool     equalequal_fn(any_t a, any_t b);
extern  bool     notequal_fn(any_t a, any_t b);
extern  any_t   inc(any_t);
extern  any_t   dec(any_t);
extern  any_t   inc_copy(any_t);
extern  any_t   dec_copy(any_t);

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
extern  any_t   lookup(any_t env, symbol_t s);
extern  any_t   lookup_local(any_t l, symbol_t s);
extern  bool    isparent(any_t p, any_t o);
extern  bool    check(any_t t, any_t o);
extern  bool    pcheck(any_t t, any_t o);
extern  bool    is_void(any_t o);
extern  any_t   location(any_t l, symbol_t s);

//       x = 17;    ->  assign(local_sym, \x, 17);
//       a.x = 42;  ->  assign(a, \x, 42);
extern  any_t   assign(any_t obj, symbol_t s, any_t newobj);

//       [x, y] = [17, 42];  ->  assign_many(local_sym, lhs, rhs);
extern  any_t   assign_many(any_t obj, tuple_t t, any_t newobj);

extern  any_t   assign_fn(any_t obj, any_t a, any_t newobj);

extern  any_t   extend(any_t this, symbol_t s, tuple_t args, 
			       any_t env, any_t value);


// 'has'
extern  bool     has(any_t obj, symbol_t s);

extern  void       rm(any_t, symbol_t s);

// will be called by prule 'include'
extern  void       include(any_t dest, any_t source);
// will be called by prule 'subscribe'
extern  void       subscribe(any_t dest, any_t interface);

//extern any_t   ls(any_t o);
//extern any_t   lsall(any_t o);

#endif
