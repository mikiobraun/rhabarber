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
//#include "method_tr.h"

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

object_t object_new(primtype_t p)
     // creates a new primitive object
{
  object_t o = ALLOC_SIZE(primtype_size(p));
  if (primtype_obj(p) == 0) {
    primtype_setobj(p, o);
    OBJ(o)->parent = 0;
  }
  else
    OBJ(o)->parent = primtype_obj(p);
  OBJ(o)->ptype = p;

  OBJ(o)->table = symtable_new();
  printdebug(" %p created of primtype %s (in object_new).\n", 
	     (void *) o, primtype_name(p));
  printdebug(" %p.parent -> %p (in object_new)\n", 
	     (void *) o, (void *) (OBJ(o)->parent));
  return o;
}


object_t object_clone(object_t parent)
     // clones an existing object, which will be 
     // the parent of the new object.
     // note that cloning does *not* imply subtyping, 
     // e.g. list(int) is not a subtype of list
{
  if (object_primtype(parent) != plain_type) {
    rha_warning("Cloning object of primitive type, content and "
		"primtype will be not inherited.\n");
  }
  object_t o = plain_new();
  OBJ(o)->parent = parent;
  printdebug(" %p.parent -> %p (in object_clone)\n", 
	     (void *) o, (void *) parent);
  return o;
}



/*
 *
 * The symbol table
 *
 */


object_t object_lookuplocal(object_t env, symbol_tr s)
{
  object_t o = 0;
  if (!env) return 0;
  if (symbol_equal_symbol(s, parent_sym)) {
    // special symbol 'parent'
    o = object_parent(env);
  }
  else {
    // look into the symbol table
    o = symtable_lookup(object_table(env), s);
  }
  return o;
}


// look up a symbol and return information on the location
object_t object_lookup(object_t env, symbol_tr s)
{
  assert(s);
  object_t o = object_lookuplocal(env, s);
  // look at other places
  if (!o && object_parent(env)) {
    o = rha_lookup(object_parent(env), s);
  }
  if (!o) {
    // not found
    printdebug("object_lookup(env=%p, s=\"%s\") failed\n", 
	       (void *) env, symbol_name(s));
  }
  return o;
}


object_t object_replace(object_t env, symbol_tr s, object_t newobj)
{
  assert(s);
  object_t o = 0;
  if (!env) return 0;

  // catch parent
  if (symbol_equal_symbol(s, parent_sym)) {
    object_setparent(env, newobj);
    return newobj;
  }

  // assign to the symbol table
  if (symtable_lookup(object_table(env), s)) 
    o = object_assign(env, s, newobj);
  // else, propagate upwards
  if (!o && object_parent(env)) 
    o = rha_replace(object_parent(env), s, newobj);
  return o;
}


object_t object_assign(object_t o, symbol_tr s, object_t v)
{
  printdebug(" %p.%s -> %p\n", (void *) o, symbol_name(s), (void *) v);
  // catch special symbols
  if (symbol_equal_symbol(s, parent_sym)) {
    object_setparent(o, v);
    return v;
  } else {
    // assign 's' to the sym table
    symtable_assign( OBJ(o)->table, s, v);
    return v;
  }
}


void object_delete(object_t o, symbol_tr s) 
{
  symtable_delete( OBJ(o)->table, s );
}

/* call a slot of an object
 *
 * This function should be used for calling slots from within C.
 */
object_t object_callslot(object_t o, string_t slotname, int narg, ...)
{
  // lookup slotname in o
  object_t slot = rha_lookup(o, symbol_new(slotname));
  if (!slot) 
    rha_error("Symbol \"%s\" not found (object_callslot)!\n", slotname);

  // check that slot is callable
  if (!rha_callable(slot))
    rha_error("Slot named \"%s\" is not callable (by object_callslot)!\n", 
	      slotname);

  // construct tuple
  tuple_tr t = tuple_new(narg + 1);
  tuple_set(t, 0, slot);
  va_list ap;
  va_start(ap, narg);
  for (int i = 0; i < narg; i++)
    tuple_set(t, i+1, va_arg(ap, object_t));
  va_end(ap);

  // call the slot with the args in t
  // note that the args are not evaluated
  return rha_call(o, t);
}


void object_snatch(object_t dest, object_t source)
{
  list_tr l = symtable_tolist(object_table(source));
  // first check whether some of the slots are already there
  { // additional scope is for the listiterator of LIST_FOREACH
    tuple_tr t;  // iter var
    bool slots_exist = false;
    list_foreach(t, l) {
      if (symtable_lookup(object_table(dest), tuple_get(t, 0)))
	slots_exist = true;
      //if(slots_exist) 
      //	rha_warning("snatch: some of the slots are overwritten.\n");
    }
  }
  // second we copy the slots
  tuple_tr t;
  list_foreach(t, l)
    object_assign(dest, tuple_get(t, 0), tuple_get(t, 1));
}


void object_match(object_t dest, object_t interface)
{
  // not implemented
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
