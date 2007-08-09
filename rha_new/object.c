/*
 * object - the main rhabarber object
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#define OBJECT_IMPLEMENTATION
#include "object.h"

// RHA
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "alloc.h"
#include "symbol_fn.h"
#include "symtable.h"
#include "object.h"
#include "debug.h"

//#include "builtin_tr.h"

struct symtable;

/*
 *
 * Creation and Member access
 *
 */

object_t new()
     // creates a new primitive object
{
  object_t o = ALLOC_SIZE(sizeof(struct rha_object));
  o->ptype = 0;
  o->table = symtable_new();
  printdebug(" %p created by object.h->new.\n", (void *) o);
  return o;
}

object_t new_t(int_t pt, object_t proto)
{
  object_t o = new();
  setptype(o, pt);
  assign(o, symbol_new("parent"), proto);
  return o;
}

object_t clone(object_t parent)
     // clones an existing object, which will be 
     // the parent of the new object.
{
  object_t o = new();
  o->ptype = parent->ptype;
  o->raw = parent->raw;
  symtable_assign( o->table, symbol_new("parent"), parent);
  printdebug(" %p.parent -> %p (in object.h->clone)\n", 
	     (void *) o, (void *) parent);
  return o;
}


int_t ptype(object_t o)
{
  return o->ptype;
}

void setptype(object_t o, int_t id)
{
  o->ptype = id;
}

object_t wrap_int(int ptype, object_t proto, int i)
{
  object_t o = new();
  setptype(o, ptype);
  o->raw.i = i;
  if (proto)
    assign(o, symbol_new("parent"), proto);    
  return o;
}

object_t wrap_float(int ptype, object_t proto, float f)
{
  object_t o = new();
  setptype(o, ptype);
  o->raw.f = f;
  if (proto)
    assign(o, symbol_new("parent"), proto);    
  return o;
}

object_t wrap_double(int ptype, object_t proto, double d)
{
  object_t o = new();
  setptype(o, ptype);
  o->raw.d = d;
  if (proto)
    assign(o, symbol_new("parent"), proto);    
  return o;
}

object_t wrap_ptr(int ptype, object_t proto, void *p)
{
  object_t o = new();
  setptype(o, ptype);
  o->raw.p = p;
  if (proto)
    assign(o, symbol_new("parent"), proto);    
  return o;
}

int unwrap_int(int pt, object_t o)
{
  assert(!pt || ptype(o) == pt);
  return o->raw.i;
}

float unwrap_float(int pt, object_t o)
{
  assert(!pt || ptype(o) == pt);
  return o->raw.f;
}

double unwrap_double(int pt, object_t o)
{
  assert(!pt || ptype(o) == pt);
  return o->raw.d;
}

void* unwrap_ptr(int pt, object_t o)
{
  assert(!pt || ptype(o) == pt);
  return o->raw.p;
}

/*
 *
 * The symbol table
 *
 */


// look up a symbol and return information on the location
object_t lookup(object_t l, symbol_t s)
{
  assert(l);

  object_t o = symtable_lookup(l->table, s);
  if (o) return o;
  // else
  //   look along the parent hierarchy
  object_t parent = symtable_lookup(l->table, parent_sym);
  if (parent) return lookup(parent, s);
  // else
  //   there is no parent
  printf("lookup for symbol %s failed!\n", symbol_name(s));
  return 0;
}


object_t assign(object_t o, symbol_t s, object_t v)
{
  printdebug(" %p.%s -> %p\n", (void *) o, symbol_name(s), (void *) v);
  // assign 's' to the sym table
  symtable_assign( o->table, s, v);
  return v;
}


void rmslot(object_t o, symbol_t s) 
{
  symtable_delete( o->table, s );
}


void include(object_t dest, object_t source)
{
  // not implemented
  assert(0);
}

// will be called by prule 'subscribe'
void subscribe(object_t dest, object_t interface)
{
  // not implemented
  assert(0);
}






