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
#include "utils.h"
#include "tuple_fn.h"
#include "list_fn.h"
#include "string_fn.h"

struct symtable;

object_t object_init(object_t root, object_t module)
{
  object_t f = lookup(module, ls_sym);
  assign(root, ls_sym, f);
  return root;
}


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
  // debug(" %p created by object.h->new.\n", (void *) o);
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
  // debug(" %p.parent -> %p (in object.h->clone)\n", (void *) o, (void *) parent);
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
  assert(ptype!=BOOL_T);
  assert(ptype!=INT_T);
  assert(ptype!=REAL_T);
  assert(ptype!=SYMBOL_T);
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
  // else look along the parent hierarchy
  object_t parent = symtable_lookup(l->table, parent_sym);
  if (parent) return lookup(parent, s);
  // else there is no parent
  return 0;
}


object_t assign(object_t o, symbol_t s, object_t v)
{
  // debug(" %p.%s -> %p\n", (void *) o, symbol_name(s), (void *) v);
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







/**********************************************************************
 * 
 * Printing objects
 *
 **********************************************************************/


string_t to_string(object_t o)
     // right now, this is a big switch, but eventually, this should
     // be done via symbol lookup :)
{
  string_t s = 0;
  if (!o) {
    // this means: undefined, i.e. evaluation failed
    return sprint("%s", "<NULL>");
  }
  else if (o==void_obj) {
    return sprint("%s", "<void>");
  }
  else {
    switch (ptype(o)) {
    case BOOL_T: {
      if (UNWRAP_BOOL(o))
	return sprint("true");
      else
	return sprint("false");
    }
    case INT_T: {
      return sprint("%d", UNWRAP_INT(o));
    }
    case SYMBOL_T: {
      symbol_t s = UNWRAP_SYMBOL(o);
      //      return sprint("%s (sym%d)", symbol_name(s), s);
      return sprint("%s", symbol_name(s));
    }
    case STRING_T: {
      s = UNWRAP_PTR(STRING_T, o);
      return sprint("\"%s\"", s); 
    }
    case REAL_T: {
      return sprint("%f", UNWRAP_REAL(o));
    }
    case TUPLE_T: {
      tuple_t t = UNWRAP_PTR(TUPLE_T, o);
      s = gc_strdup("(");
      int tlen = tuple_len(t);
      for(int i = 0; i < tlen; i++) {
	if (i > 0) s = string_append(s, ", ");
	s = string_append(s, to_string(tuple_get(t, i)));
      }
      return string_append(s, ")");
    }
    case LIST_T: {
      s = gc_strdup("[");
      list_t l = UNWRAP_PTR(LIST_T, o);
      int i; 
      list_it it;
      for(list_begin(l, &it), i = 0; !list_done(&it); list_next(&it), i++) {
	if (i > 0) s = string_append(s, ", ");
	s = string_append(s, to_string(list_get(&it)));
      }
      return string_append(s, "]");
    }
    case FN_T: {
      fn_t f = UNWRAP_PTR(FN_T, o);
      s = sprint("<fn narg=%d, ", f->narg);
      for(int i = 0; i < f->narg; i++) {
	if (i > 0)
	  s = sprint(s, ", ");
	s = string_append(s, ptype_name(f->argptypes[i]));
      }
      return string_append(s, ">");
    }
    default:
      return sprint("<addr=%p>", (void*) o);
    }
  }
}

void print_fn(object_t o)
{
  fprintf(stdout, "%s", to_string(o));
}



list_t ls(object_t o)
{
  return symtable_tolist(o->table);
}
