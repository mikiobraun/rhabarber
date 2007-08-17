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
#include "eval.h"
#include "symbol_fn.h"
#include "symtable.h"
#include "object.h"
#include "debug.h"
#include "utils.h"
#include "messages.h"
#include "tuple_fn.h"
#include "list_fn.h"
#include "string_fn.h"
#include "excp.h"

struct symtable;

object_t object_empty_excp = 0;

void object_init(object_t root, object_t module)
{
  object_empty_excp = excp_new("accessing a primitive prototype is not allowed\n");
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
  o->ptype = OBJECT_T;
  o->table = symtable_new();
  o->raw.p = 0;
  // debug(" %p created by object.h->new.\n", (void *) o);
  return o;
}

object_t new_t(int_t pt, object_t proto)
{
  object_t o = new();
  setptype(o, pt);
  if (proto)
    assign(o, symbol_new("parent"), proto);    
  return o;
}

object_t copy(object_t other)
{
  object_t o = new();
  o->ptype = other->ptype;
  o->table = symtable_copy(other->table);
  o->raw   = other->raw;
  return o;
}

address_t addr(object_t o)
{
  return (address_t) o;
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
  object_t o = new_t(ptype, proto);
  o->raw.i = i;
  return o;
}

object_t wrap_float(int ptype, object_t proto, float f)
{
  object_t o = new_t(ptype, proto);
  o->raw.f = f;
  return o;
}

object_t wrap_double(int ptype, object_t proto, double d)
{
  object_t o = new_t(ptype, proto);
  o->raw.d = d;
  return o;
}

object_t wrap_ptr(int ptype, object_t proto, void *p)
{
  assert(ptype!=BOOL_T);
  assert(ptype!=INT_T);
  assert(ptype!=REAL_T);
  assert(ptype!=SYMBOL_T);
  object_t o = new_t(ptype, proto);
  o->raw.p = p;
  return o;
}

int unwrap_int(int pt, object_t o)
{
  assert(pt != OBJECT_T || ptype(o) == pt);
  return o->raw.i;
}

float unwrap_float(int pt, object_t o)
{
  assert(pt != OBJECT_T || ptype(o) == pt);
  return o->raw.f;
}

double unwrap_double(int pt, object_t o)
{
  assert(pt != OBJECT_T || ptype(o) == pt);
  return o->raw.d;
}

void *unwrap_ptr(int pt, object_t o)
{
  assert(pt != OBJECT_T || ptype(o) == pt);
  void *p = o->raw.p;
  if (!p) 
    throw(object_empty_excp);
  return p;
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
  return 0; // ZERO meaning "not found"
}

bool_t has(object_t obj, symbol_t s)
{
  if (lookup(obj, s))
    return true;
  else
    return false;
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
    case OBJECT_T: {
      return sprint("<addr=%p>", (void *) addr(o));
    }
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
    case REAL_T: {
      return sprint("%f", UNWRAP_REAL(o));
    }
    default:
      // content is stored in o->raw.p
      if (!o->raw.p)
	// we are accessing a protoype
	return "<primitive prototype>";
      // we can safely access the raw content
      switch (ptype(o)) {
      case STRING_T: {
	s = UNWRAP_PTR(STRING_T, o);
	return sprint("\"%s\"", s); 
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
      case FUNCTION_T: {
	function_t f = UNWRAP_PTR(FUNCTION_T, o);
	s = sprint("<fn (");
	for(int i = 0; i < f->narg; i++) {
	  if (i > 0) s = string_append(s, ", ");
	  s = string_append(s, ptype_names[f->argptypes[i]]);
	}
	return string_append(s, ")>");
      }
      case ADDRESS_T: {
	return sprint("%p", (void *) UNWRAP_PTR(ADDRESS_T, o));
      }
      default:
	return sprint("<UNKNOWN ptype: addr=%p>", (void *) addr(o));
      }
    }
  }
}


void print_fn(object_t o)
{
  fprintf(stdout, "%s\n", to_string(o));
}



void ls(object_t o)
{
  symtable_print(o->table);
}

object_t inc(object_t o)
{
  if (ptype(o) == INT_T) {
    o->raw.i++;
    return o;
  }
  rha_error("'inc' is currently only implemented for integers");
  assert(1==0);
}

object_t dec(object_t o)
{
  if (ptype(o) == INT_T) {
    o->raw.i--;
    return o;
  }
  rha_error("'dec' is currently only implemented for integers");
  assert(1==0);
}

object_t inc_copy(object_t o)
{
  if (ptype(o) == INT_T) {
    object_t other = copy(o);
    o->raw.i++;
    return other;
  }
  rha_error("'inc_copy' is currently only implemented for integers");
  assert(1==0);
}

object_t dec_copy(object_t o)
{
  if (ptype(o) == INT_T) {
    object_t other = copy(o);
    o->raw.i--;
    return other;
  }
  rha_error("'dec_copy' is currently only implemented for integers");
  assert(1==0);
}


bool_t equalequal_fn(object_t a, object_t b)
{
  if ((ptype(a) == INT_T) && (ptype(b) == INT_T))
    return UNWRAP_INT(a) == UNWRAP_INT(b);
  else
    return a == b;
}

bool_t notequal_fn(object_t a, object_t b)
{
  if ((ptype(a) == INT_T) && (ptype(b) == INT_T))
    return UNWRAP_INT(a) != UNWRAP_INT(b);
  else
    return a != b;
}

bool_t in_fn(object_t a, object_t b)
{
  rha_error("element check for iterables is not yet implemented");
  assert(1==0);
}

int_t neg_fn(object_t a)
{
  if (ptype(a) == INT_T)
    return -UNWRAP_INT(a);
  rha_error("'neg_fn' is currently only implemented for integers");
  assert(1==0);
}

#define SIMPLE_FN(returntype, name, symbol)				\
  returntype name ## _fn(object_t a, object_t b)			\
  {									\
    if ((ptype(a) == INT_T) && (ptype(b) == INT_T))			\
      return UNWRAP_INT(a) symbol UNWRAP_INT(b);			\
    rha_error("'" #name "' is currently only implemented for integers"); \
    assert(1==0);							\
  }									

SIMPLE_FN(bool_t, less, <)
SIMPLE_FN(bool_t, lessequal, <=)
SIMPLE_FN(bool_t, greater, >)
SIMPLE_FN(bool_t, greaterequal, >=)
SIMPLE_FN(int_t, plus, +)
SIMPLE_FN(int_t, minus, -)
SIMPLE_FN(int_t, times, *)
SIMPLE_FN(int_t, divide, /)
