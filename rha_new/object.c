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
#include <string.h>
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

void object_init(any_t root, any_t module)
{
  // nothing here
}


/*
 *
 * Creation and Member access
 *
 */

any_t new()
     // creates a new primitive object
{
  any_t o = ALLOC_SIZE(sizeof(struct rha_object));
  o->ptype = ANY_T;
  o->table = symtable_new();
  o->raw.p = 0;
  // debug(" %p created by object.h->new.\n", (void *) o);
  return o;
}

any_t create_pt(int_t pt)
{
  any_t o = new();
  setptype(o, pt);
  return o;
}

any_t new_pt(int_t pt)
{
  any_t o = new();
  setptype(o, pt);
  assign(o, parent_sym, prototypes[pt]);
  return o;
}

any_t b_copy_pt(tuple_t t)
{
  return copy_pt(tuple_get(t, 1));  
}

any_t copy_pt(any_t other)
{
  // note that symbol table information is lost
  // thus this function is not intended to copy ANY_T stuff
  assert(ptype(other) != ANY_T);
  any_t o = new();
  o->ptype   = other->ptype;
  o->raw     = other->raw;
  assign(o, parent_sym, prototypes[o->ptype]);
  return o;
}

address_t addr(any_t o)
{
  return (address_t) o;
}

any_t clone(any_t parent)
     // clones an existing object, which will be 
     // the parent of the new object.
{
  any_t o = new();
  o->ptype = parent->ptype;
  o->raw = parent->raw;
  symtable_assign( o->table, parent_sym, parent);
  // debug(" %p.parent -> %p (in object.h->clone)\n", (void *) o, (void *) parent);
  return o;
}


int_t ptype(any_t o)
{
  if (!o) return 0; // == VOID_T // see rha_types.h
  return o->ptype;
}

string_t ptypename(any_t o)
{
  return ptype_names[ptype(o)];
}

void setptype(any_t o, int_t id)
{
  o->ptype = id;
}

any_t wrap_int(int pt, int i)
{
  assert(pt==BOOL_T || pt==INT_T || pt==SYMBOL_T);
  any_t o = new_pt(pt);
  o->raw.i = i;
  return o;
}

any_t wrap_float(int pt, float d)
{
  assert(pt==REAL_T);
  any_t o = new_pt(pt);
  o->raw.d = d;
  return o;
}

any_t wrap_double(int pt, double d)
{
  assert(pt==REAL_T);
  any_t o = new_pt(pt);
  o->raw.d = d;
  return o;
}

any_t wrap_builtin(int pt, builtin_t b)
{
  assert(pt==BUILTIN_T);
  any_t o = new_pt(pt);
  o->raw.b = b;
  return o;
}

any_t wrap_ptr(int pt, void *p)
{
  assert(pt!=BOOL_T);
  assert(pt!=INT_T);
  assert(pt!=SYMBOL_T);
  assert(pt!=REAL_T);
  any_t o = new_pt(pt);
  o->raw.p = p;
  return o;
}

any_t wrap(int ptype, ...)
{
  va_list ap;
  any_t o = new_pt(ptype);
  switch (ptype) {
  case BOOL_T:
  case INT_T:
  case SYMBOL_T:
    o->raw.i = va_arg(ap, int);
    break;
  case REAL_T:
    o->raw.d = va_arg(ap, double);
    break;
  default:
    o->raw.p = va_arg(ap, void *);
  }
  va_end(ap);
  return o;
}

int unwrap_int(int pt, any_t o)
{
  assert(pt==INT_T || pt==BOOL_T || pt==SYMBOL_T);
  assert(pt==ptype(o));
  return o->raw.i;
}

double unwrap_double(int pt, any_t o)
{
  assert(pt==REAL_T);
  assert(pt==ptype(o));
  return o->raw.d;
}

builtin_t unwrap_builtin(int pt, any_t o)
{
  assert(pt==BUILTIN_T);
  assert(pt==ptype(o));
  return o->raw.b;
}

void *unwrap_ptr(int pt, any_t o)
{
  assert(pt != ANY_T || ptype(o) == pt);
  void *p = o->raw.p;
  // might be zero
  return p;
}

int uwi(int pt, any_t o, string_t msg)
{
  if (!o || ptype(o) != pt)
    rha_error(msg);
  return unwrap_int(pt, o);
}

int uwd(int pt, any_t o, string_t msg)
{
  if (!o || ptype(o) != pt)
    rha_error(msg);
  return unwrap_double(pt, o);
}

void *uw(int pt, any_t o, string_t msg)
{
  if (!o || ptype(o) != pt)
    rha_error(msg);
  void *p = o->raw.p;
  return p;
}


/*
 *
 * Creating builtin functions
 *
 */
any_t vcreate_builtin(builtin_t code, bool_t varargs, int narg, va_list args)
{
  // built a signature to be passed to 'create_fn_data_entry'
  tuple_t signature = 0;
  if (varargs) {
    // reserve a spot for the ellipsis
    signature = tuple_new(narg+1);
    tuple_set(signature, narg, create_pattern(1, WRAP_SYMBOL(symbol_new("..."))));
  }
  else
    signature = tuple_new(narg);
  for (int i=0; i<narg; i++) {
    int_t pt = va_arg(args, int_t);
    // we assume that the typeobject for ANY_T is zero
    // this allows us later to avoid testing at all!
    assert(pt!=ANY_T || typeobjects[pt]==0);
    // note that 'theliteral' of the pattern is void
    // note that for pt==ANY_T also 'thetype' is void
    tuple_set(signature, i, create_pattern(2, 0, typeobjects[pt]));
  }

  // create a new object and return it
  return fn_fn(0, signature, WRAP_BUILTIN(code));
}


any_t create_builtin(builtin_t code, bool_t varargs, int narg, ...)
{
  // read out the argument types
  va_list args;
  va_start(args, narg);
  any_t f = vcreate_builtin(code, varargs, narg, args);
  va_end(args);

  return f;
}


/*
 *
 * The symbol table
 *
 */


// look up a symbol and return information on the location
any_t lookup(any_t l, symbol_t s)
{
  if (!l) return 0;

  any_t o = symtable_lookup(l->table, s);
  if (o) return o;
  // else look along the parent hierarchy
  any_t parent = symtable_lookup(l->table, parent_sym);
  if (parent) return lookup(parent, s);
  // else there is no parent
  return 0; // ZERO meaning "not found"
}

// look up a symbol only locally, i.e. don't follow parents
any_t lookup_local(any_t l, symbol_t s)
{
  if (!l)
    rha_error("void has no slots");
  any_t o = symtable_lookup(l->table, s);
  if (!o)
    rha_error("local lookup of symbol '%o' failed", WRAP_SYMBOL(s));
  return o;
}

// look up a symbol and return the location
any_t location(any_t l, symbol_t s)
{
  if (!l) return 0;
  if (symtable_lookup(l->table, s)) return l;
  // else look along the parent hierarchy
  any_t parent = symtable_lookup(l->table, parent_sym);
  if (parent) return location(parent, s);
  // else there is no parent
  return 0; // ZERO meaning "not found"
}

bool_t has(any_t obj, symbol_t s)
{
  // note that 'void' doesn't have any slots
  if (lookup(obj, s))
    return true;
  else
    return false;
}


any_t assign(any_t o, symbol_t s, any_t v)
{
  // debug(" %p.%s -> %p\n", (void *) o, symbol_name(s), (void *) v);
  // assign 's' to the sym table
  if(!v) v = void_obj;
  symtable_assign(o->table, s, v);
  return v;
}


bool_t is_void(any_t o)
{
  return o==0;
}

any_t extend(any_t this, symbol_t s, tuple_t signature, 
		any_t env, any_t rhs)
{
  // extend object behind 's' 
  //             in scope 'this'
  //          for context 'signature'
  //             by value 'rhs'
  //   with lexical scope 'env'

  // (1) does 's' exists already?  if not, create a new function.
  any_t obj = lookup(this, s);
  if (!obj) {
    assign(this, s, fn_fn(env, signature, rhs));
    return 0;
  }
  // else extend 'obj'

  // (2) does 'obj' have its own 'extend' mechanism (like tuples)?
  if (has(obj, extend_sym)) {
    // ignore the rest of this function and call it!
    return callslot(obj, extend_sym, 3, WRAP_PTR(TUPLE_T, signature), env, rhs);
  }

  // (3) is 'obj' itself already a function?
  // if an ancestor of 'obj' is already a function, 'obj' will be a
  // new function
  list_t fn_data_l = 0;
  any_t ancestor = location(obj, fn_data_sym);  // the ancestor
                                                // defining a function
  if (ancestor != obj) {
    if (ancestor)
      rha_warning("creating in %o a new function which will hide the "
		  "function defined by its ancestor %o", obj, ancestor);
    fn_data_l = list_new();
  }
  else {
    any_t fn_data = lookup(obj, fn_data_sym);
    if (ptype(fn_data)!=LIST_T
	|| list_len(fn_data_l=UNWRAP_PTR(LIST_T, fn_data)) == 0)
      rha_error("(eval) %o has a faulty 'fn_data' slot", obj);
    fn_data_l = UNWRAP_PTR(LIST_T, fn_data);
  }
  any_t entry = create_fn_data_entry(env, signature, rhs);
  fn_data_l = list_insert_sorted(fn_data_l, entry, &fn_data_entry_lessthan);
  any_t fn_data = WRAP_PTR(LIST_T, fn_data_l);
  assign(fn_data, parent_sym, fn_data_proto);
  assign(obj, fn_data_sym, fn_data);
  return 0;
}


void rm(any_t o, symbol_t s) 
{
  symtable_delete( o->table, s );
}


void include(any_t dest, any_t source)
{
  // not implemented
  assert(0);
}

// will be called by prule 'subscribe'
void subscribe(any_t dest, any_t interface)
{
  // not implemented
  assert(0);
}







/**********************************************************************
 * 
 * Printing objects
 *
 **********************************************************************/

string_t to_string(any_t o)
{
  // VOID
  if (!o || o==void_obj) {
    // in C it is zero, in rhabarber called void
    return sprint("%s", "<void>");
  }
  // PRIMITIVE PROTOTYPES
  int pt = ptype(o);
  if (o==prototypes[pt])
    return sprint("<%s prototype>", ptype_names[pt]);

  // SOMETHING WITH SLOT 'to_string'
  if (has(o, string_sym)) {
    any_t s_o = callslot(o, string_sym, 0);
    if (!s_o || ptype(s_o) != STRING_T)
      rha_error("(to_string) slot 'string' found but does not create a string");
    return UNWRAP_PTR(STRING_T, s_o);
  }

  // NO SLOT 'string'
  return sprint("<object@%p>", (void *) addr(o));
}

string_t builtin_to_string(builtin_t b)
{
  return sprint("<builtin@%p>", b);
}

string_t address_to_string(address_t a)
{
  return sprint("%p", (void *) a);
}

string_t to_string_only_in_c(any_t o)
     // right now, this is a big switch, but eventually, this should
     // be done via symbol lookup :)
{
  string_t s = 0;
  if (!o) {
    // in C it is zero, in rhabarber called void
    return sprint("%s", "<void>");
  }
  else {
    int pt = ptype(o);
    if (o==prototypes[pt])
      return sprint("<%s prototype>", ptype_names[pt]);
    // else something more interesting
    switch (pt) {
    case ANY_T: {
      return sprint("<object@%p>", (void *) addr(o));
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
      return sprint("%s", symbol_name(s));
    }
    case REAL_T: {
      return sprint("%f", UNWRAP_REAL(o));
    }
    default:
      // content is stored in o->raw.p
      assert(o->raw.p);
      // we catched the case (o->raw.p==0) above, since this can only
      // happen primitive prototypes, thus we can safely access the
      // raw content
      switch (pt) {
      case STRING_T: {
	s = UNWRAP_PTR(STRING_T, o);
	return sprint("%s", s); 
      }
      case TUPLE_T: {
	tuple_t t = UNWRAP_PTR(TUPLE_T, o);
	s = gc_strdup("(");
	int tlen = tuple_len(t);
	for(int i = 0; i < tlen; i++) {
	  if (i > 0) s = string_concat(s, " ");
	  s = string_concat(s, to_string_only_in_c(tuple_get(t, i)));
	}
	return string_concat(s, ")");
      }
      case LIST_T: {
	s = gc_strdup("[");
	list_t l = UNWRAP_PTR(LIST_T, o);
	int i; 
	list_it_t it;
	for(it = list_begin(l), i = 0; !list_done(it); list_next(it), i++) {
	  if (i > 0) s = string_concat(s, ", ");
	  s = string_concat(s, to_string_only_in_c(list_get(it)));
	}
	return string_concat(s, "]");
      }
      case ADDRESS_T: {
	return sprint("%p", (void *) UNWRAP_PTR(ADDRESS_T, o));
      }
      case BUILTIN_T: {
	return sprint("<builtin@%p>", (void *) UNWRAP_PTR(BUILTIN_T, o));
      }
      default:
	return sprint("<UNKNOWN ptype: addr=%p>", (void *) addr(o));
      }
    }
  }
}

void vprint_fn(tuple_t args)
{
  for (int i=0; i<tuple_len(args); i++)
    fprintf(stdout, "%s", to_string(tuple_get(args, i)));
  fprintf(stdout, "\n");
}

void print_fn(int_t narg, ...)
{
  va_list ap;
  va_start(ap, narg);
  tuple_t args = tuple_new(narg);
  for (int i = 0; i < narg; i++)
    tuple_set(args, i,va_arg(ap, any_t));
  va_end(ap);
  
  vprint_fn(args);
}



void ls(any_t o)
{
  symtable_print(o->table); return;
  while( (o = lookup(o, parent_sym)) ) {
    printf("^\n");
    symtable_print(o->table);
  }
}


bool_t equalequal_fn(any_t a, any_t b)
{
  if ((ptype(a) == ADDRESS_T) && (ptype(b) == ADDRESS_T))
    return UNWRAP_PTR(ADDRESS_T, a) == UNWRAP_PTR(ADDRESS_T, b);
  if ((ptype(a) == SYMBOL_T) && (ptype(b) == SYMBOL_T))
    return UNWRAP_PTR(SYMBOL_T, a) == UNWRAP_PTR(SYMBOL_T, b);
  if ((ptype(a) == BOOL_T) && (ptype(b) == BOOL_T))
    return UNWRAP_BOOL(a) == UNWRAP_BOOL(b);
  if ((ptype(a) == INT_T) && (ptype(b) == INT_T))
    return UNWRAP_INT(a) == UNWRAP_INT(b);
  if ((ptype(a) == REAL_T) && (ptype(b) == INT_T))
    return UNWRAP_REAL(a) == (real_t) UNWRAP_INT(b);
  if ((ptype(a) == INT_T) && (ptype(b) == REAL_T))
    return (real_t) UNWRAP_INT(a) == UNWRAP_REAL(b);
  if ((ptype(a) == REAL_T) && (ptype(b) == REAL_T))
    return UNWRAP_REAL(a) == UNWRAP_REAL(b);
  else if ((ptype(a) == STRING_T) && (ptype(b) == STRING_T))
    return 0==strcmp(UNWRAP_PTR(STRING_T, a), UNWRAP_PTR(STRING_T, b));
  else
    return a == b;
}

bool_t notequal_fn(any_t a, any_t b)
{
  return !equalequal_fn(a, b);
}


any_t inc(any_t o)
{
  if (ptype(o) == INT_T) {
    o->raw.i++;
    return o;
  }
  rha_error("'inc' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}

any_t dec(any_t o)
{
  if (ptype(o) == INT_T) {
    o->raw.i--;
    return o;
  }
  rha_error("'dec' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}

any_t inc_copy(any_t o)
{
  if (ptype(o) == INT_T) {
    any_t other = copy_pt(o);
    o->raw.i++;
    return other;
  }
  rha_error("'inc_copy' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}

any_t dec_copy(any_t o)
{
  if (ptype(o) == INT_T) {
    any_t other = copy_pt(o);
    o->raw.i--;
    return other;
  }
  rha_error("'dec_copy' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}


// 'check' does a type check by looking along the parent hierarchy
bool_t check(any_t t, any_t o)
{
  any_t proto = lookup(t, proto_sym);
  if (!proto)
    rha_error("(check) object %o doesn't have a slot 'proto'", t);
  do {
    if (addr(o) == addr(proto))
      return true;
  } while ((o = lookup(o, parent_sym)));
  return false;
}

// 'pcheck' does a primitive type check which is sufficient (and much
// faster) for ptypes
bool_t pcheck(any_t t, any_t o)
{
  any_t proto = lookup(t, proto_sym);
  if (!proto)
    rha_error("(check) object %o doesn't have a slot 'proto'", t);
  return ptype(proto) == ptype(o);
}
