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
#include "alloc.h"
#include "primtype.h"
//#define DEBUG
#include "debug.h"
#include "listobject.h"
#include "symtable.h"
#include "eval.h"
#include "messages.h"
#include "utils.h"

//#include "builtin_tr.h"
#include "core.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "list_tr.h"
#include "function_tr.h"
#include "plain_tr.h"

struct symtable;

#define OBJ(x) ((struct rha_object *)(x))

#ifdef USE_INLINES
#define INLINE inline
#else
#define INLINE
#endif
#include "object_inline.c"


/*
 *
 * Creation and Member access
 *
 */

object_t new()
     // creates a new primitive object
{
  object_t o = ALLOC_SIZE(size_of(struct rha_object));
  OBJ(o)->ptype = 0;
  OBJ(o)->table = symtable_new();
  OBJ(o)->raw = 0;
  printdebug(" %p created by object.h->new.\n", (void *) o);
  return o;
}


object_t clone(object_t parent)
     // clones an existing object, which will be 
     // the parent of the new object.
{
  object_t o = new();
  OBJ(o)->ptype = parent->ptype;
  OBJ(o)->raw = parent->raw;
  symtable_assign( OBJ(o)->table, parent_sym, parent);
  printdebug(" %p.parent -> %p (in object.h->clone)\n", 
	     (void *) o, (void *) parent);
  return o;
}


int_t ptype(object_t o)
{
  return OBJ(o)->ptype;
}

void *raw(object_t o)
{
  return OBJ(o)->raw;
}


/*
 *
 * The symbol table
 *
 */


// look up a symbol and return information on the location
object_t lookup(object_t o, symbol_t s)
{
  assert(s);
  object_t o = symtable_lookup(OBJ(o)->table, s);
  if (o) return o;
  // else
  //   look along the parent hierarchy
  object_t parent = symtable_lookup(OBJ(o)->table, parent_sym);
  if (parent) return lookup(parent, s);
  // else
  //   there is no parent
  printdebug("lookup(o=%p, s=\"%s\") failed\n", 
	     (void *) o, symbol_name(s));
  return 0;
}


object_t assign_fn(object_t o, symbol_t s, object_t v)
{
  printdebug(" %p.%s -> %p\n", (void *) o, symbol_name(s), (void *) v);
  // assign 's' to the sym table
  symtable_assign( OBJ(o)->table, s, v);
  return v;
}


void rmslot(object_t o, symbol_t s) 
{
  symtable_delete( OBJ(o)->table, s );
}


void_t     include_fn(object_t dest, object_t source)
{
  // not implemented
  assert(0);
}
// will be called by prule 'subscribe'
void_t     subscribe_fn(object_t dest, object_t interface)
{
  // not implemented
  assert(0);
}




void object_print(object_t o)
     // right now, this is a big switch, but eventually, this should
     // be done via symbol lookup :)
{
  if (!o) {
    fprintf(stdout, "nil");
  }
  else if( object_primtype(o) == 0 ) {
    fprintf(stdout, "[null]");
  }
  else {
    fprintf(stdout, "%s", object_to_string(o));
  }
}


//
// the following function are for calling rhabarber functionality from C
// 

string_t object_to_string(object_t o)
{
  if (!o)
    return "nil";
  else if (rha_lookup(o, symbol_new("to_string"))) {
    return sprint(string_get(object_callslot(o, "to_string", 0)));
  }
  else {
    object_t name = rha_lookup(o, symbol_new("name"));
    if (!name) name = rha_lookup(o, symbol_new("typename"));
    return sprint("[object %s at %p]",  object_to_string(name), o);
  }
}


string_t object_name(object_t o)
{
  if (!o)
    return "nil";
  else {
    object_t content;
    string_t name = gc_strdup("");
    symbol_tr name_sym = symbol_new("name");
    while (!(content = symtable_lookup(OBJ(o)->table, name_sym))) {
      o = object_parent(o);
      if (!o) break;
      name = string_plus_string(name, "child of ");
    }
    if (content) {
      if (HAS_TYPE(string, content)) {
	return string_plus_string(name, string_get(content));
      }
      else if (HAS_TYPE(symbol, content)) {
	return string_plus_string(name, symbol_name(content));
      }
    }
    return "no-name";
  }
}


void object_collectslots(object_t o, list_t slots)
{
  list_t l = symtable_tolist(object_table(o));

  object_t x;
  list_foreach(x, l)
    list_append(slots, tuple_get(x, 0));

  if(object_parent(o))
    object_collectslots(object_parent(o), slots);
}


list_t object_allslots(object_t o)
{
  if(!o) return 0;
  list_t slots = list_new();
  object_collectslots(o, slots);
  return slots;
}


list_t object_slots(object_t o)
{
  if(!o) return 0;

  list_t slots = list_new();

  object_t x;
  list_foreach(x, symtable_tolist(object_table(o)))
    list_append(slots, tuple_get(x, 0));

  return slots;
}


string_t object_typename(object_t obj)
{
  object_t tn = rha_lookup(obj, typename_sym);
  if(!tn || !HAS_TYPE(string, tn))
    return "unknown type";
  else
    return string_get(tn);
}
