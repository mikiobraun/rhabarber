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

any_t new(void)
     // creates a new primitive object
{
  any_t o = ALLOC_SIZE(sizeof(struct rha_object));
  o->ptype = RHA_any_t;
  o->table = symtable_new();
  o->raw.p = 0;
  // debug(" %p created by object.h->new.\n", (void *) o);
  return o;
}

any_t create_pt(int pt)
{
  any_t o = new();
  setptype(o, pt);
  return o;
}

any_t new_pt(int pt)
{
  any_t o = new();
  setptype(o, pt);
  assign(o, parent_sym, prototypes[pt]);
  return o;
}

any_t copy_pt(any_t other)
{
  // note that symbol table information is lost
  // thus this function is not intended to copy ANY_T stuff
  assert(ptype(other) != RHA_any_t);
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


int ptype(any_t o)
{
  if (!o) return 0; // == RHA_void // see rha_types.h
  return o->ptype;
}

string_t ptypename(any_t o)
{
  return ptype_names[ptype(o)];
}

void setptype(any_t o, int id)
{
  o->ptype = id;
}

any_t wrap_int(int pt, int i)
{
  assert(pt==RHA_bool || pt==RHA_int || pt==RHA_symbol_t);
  any_t o = new_pt(pt);
  o->raw.i = i;
  return o;
}

any_t wrap_double(int pt, double d)
{
  assert(pt==RHA_real_t);
  any_t o = new_pt(pt);
  o->raw.d = d;
  return o;
}

any_t wrap_builtin(int pt, builtin_t b)
{
  assert(pt==RHA_builtin_t);
  any_t o = new_pt(pt);
  o->raw.b = b;
  return o;
}

any_t wrap_ptr(int pt, void *p)
{
  assert(pt!=RHA_bool);
  assert(pt!=RHA_int);
  assert(pt!=RHA_symbol_t);
  assert(pt!=RHA_real_t);
  any_t o = new_pt(pt);
  o->raw.p = p;
  return o;
}

any_t wrap(int ptype, ...)
{
  va_list ap;
  any_t o = new_pt(ptype);
  switch (ptype) {
  case RHA_bool:
  case RHA_int:
  case RHA_symbol_t:
    o->raw.i = va_arg(ap, int);
    break;
  case RHA_real_t:
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
  assert(pt==RHA_int || pt==RHA_bool || pt==RHA_symbol_t);
  assert(pt==ptype(o));
  return o->raw.i;
}

double unwrap_double(int pt, any_t o)
{
  assert(pt==RHA_real_t);
  assert(pt==ptype(o));
  return o->raw.d;
}

builtin_t unwrap_builtin(int pt, any_t o)
{
  assert(pt==RHA_builtin_t);
  assert(pt==ptype(o));
  builtin_t b = o->raw.b;
  if (!b)
    rha_error("can not access builtin, are you trying to use the builtin prototype?");
  return b;
}

void *unwrap_ptr(int pt, any_t o)
{
  assert(pt != RHA_any_t || ptype(o) == pt);
  void *p = o->raw.p;
  // might be zero
  if (!p)
    rha_error("can not access object, are you directly accessing a prototype?");
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
any_t vcreate_builtin(builtin_t code, bool varargs, int narg, va_list args)
{
  // built a signature to be passed to 'create_fn_data_entry'
  tuple_t signature = 0;
  if (varargs) {
    // reserve a spot for the ellipsis
    signature = tuple_new(narg+1);
    tuple_set(signature, narg, WRAP_SYMBOL(symbol_new("...")));
  }
  else
    signature = tuple_new(narg);
  for (int i=0; i<narg; i++) {
    int pt = va_arg(args, int);
    // we assume that the typeobject for ANY_T is zero
    // this allows us later to avoid testing at all!
    assert(pt!=RHA_any_t || typeobjects[pt]==0);
    // note that 'theliteral' of the pattern is void
    // note that for pt==ANY_T also 'thetype' is void
    
    tuple_set(signature, i, create_prepattern(0, quoted(typeobjects[pt])));
  }

  // create a new object and return it
  return fn_fn(0, signature, WRAP_BUILTIN(code));
}


any_t create_builtin(builtin_t code, bool varargs, int narg, ...)
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

bool has(any_t obj, symbol_t s)
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

any_t assign_many(any_t o, tuple_t t, any_t v)
{
  // assign several symbols at once
  
  // check that all entries in the tuple are symbols
  int tlen = tuple_len(t);
  for (int i = 0; i < tlen; i++)
    if (ptype(tuple_get(t, i)) != RHA_symbol_t)
      rha_error("assign_many must be called with tuple of symbols");
  
  // the RHS must be iterable
  any_t iter = callslot(v, iter_sym, 0);
  int i = 0;
  while (!UNWRAP_BOOL(callslot(iter, done_sym, 0))) {
    if (i >= tlen) {
      rha_warning("some return values were not assigned");
      break;
    }
    assign(o, UNWRAP_SYMBOL(tuple_get(t, i++)), 
	   callslot(iter, get_sym, 0));
    callslot(iter, next_sym, 0);
  }
  if (i < tlen) {
    rha_error("too many return values requested");
  }
  return v;
}


any_t assign_fn(any_t obj, any_t a, any_t newobj)
{
  if (ptype(a) == RHA_symbol_t)
    return assign(obj, UNWRAP_SYMBOL(a), newobj);
  else if (ptype(a) == RHA_tuple_t)
    return assign_many(obj, UNWRAP_PTR(RHA_tuple_t, a), newobj);
  else
    rha_error("can't assign to %o", a);
  assert(false);
}

bool is_void(any_t o)
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
    // but first evaluate the entries of the signature
    int siglen = tuple_len(signature);
    for (int i = 0; i < siglen; i++)
      tuple_set(signature, i, eval(this, tuple_get(signature, i)));
    return callslot(obj, extend_sym, 3, WRAP_PTR(RHA_tuple_t, signature), env, rhs);
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
    if (ptype(fn_data)!=RHA_list_t
	|| list_len(fn_data_l=UNWRAP_PTR(RHA_list_t, fn_data)) == 0)
      rha_error("(eval) %o has a faulty 'fn_data' slot", obj);
    fn_data_l = UNWRAP_PTR(RHA_list_t, fn_data);
  }
  any_t entry = create_fn_data_entry(env, signature, rhs);
  fn_data_l = list_insert_sorted(fn_data_l, entry, &fn_data_entry_lessthan);
  any_t fn_data = WRAP_PTR(RHA_list_t, fn_data_l);
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
  symbol_t string_sym = symbol_new("string");
  if (has(o, string_sym)) {
    any_t s_o = callslot(o, string_sym, 0);
    if (!s_o || ptype(s_o) != RHA_string_t)
      rha_error("(to_string) slot 'string' found but does not create a string");
    return UNWRAP_PTR(RHA_string_t, s_o);
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
    case RHA_any_t: {
      return sprint("<object@%p>", (void *) addr(o));
    }
    case RHA_bool: {
      if (UNWRAP_BOOL(o))
	return sprint("true");
      else
	return sprint("false");
    }
    case RHA_int: {
      return sprint("%d", UNWRAP_INT(o));
    }
    case RHA_symbol_t: {
      symbol_t s = UNWRAP_SYMBOL(o);
      return sprint("%s", symbol_name(s));
    }
    case RHA_real_t: {
      return sprint("%f", UNWRAP_REAL(o));
    }
    default:
      // content is stored in o->raw.p
      assert(o->raw.p);
      // we catched the case (o->raw.p==0) above, since this can only
      // happen primitive prototypes, thus we can safely access the
      // raw content
      switch (pt) {
      case RHA_string_t: {
	s = UNWRAP_PTR(RHA_string_t, o);
	return sprint("%s", s); 
      }
      case RHA_tuple_t: {
	tuple_t t = UNWRAP_PTR(RHA_tuple_t, o);
	s = gc_strdup("(");
	int tlen = tuple_len(t);
	for(int i = 0; i < tlen; i++) {
	  if (i > 0) s = string_concat(s, " ");
	  s = string_concat(s, to_string_only_in_c(tuple_get(t, i)));
	}
	return string_concat(s, ")");
      }
      case RHA_list_t: {
	s = gc_strdup("[");
	list_t l = UNWRAP_PTR(RHA_list_t, o);
	int i; 
	list_it_t it;
	for(it = list_begin(l), i = 0; !list_done(it); list_next(it), i++) {
	  if (i > 0) s = string_concat(s, ", ");
	  s = string_concat(s, to_string_only_in_c(list_get(it)));
	}
	return string_concat(s, "]");
      }
      case RHA_address_t: {
	return sprint("%p", (void *) UNWRAP_PTR(RHA_address_t, o));
      }
      case RHA_builtin_t: {
	return sprint("<builtin@%p>", (void *) UNWRAP_PTR(RHA_builtin_t, o));
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

void print_fn(int narg, ...)
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


bool equalequal_fn(any_t a, any_t b)
{
  if ((ptype(a) == RHA_address_t) && (ptype(b) == RHA_address_t))
    return UNWRAP_PTR(RHA_address_t, a) == UNWRAP_PTR(RHA_address_t, b);
  if ((ptype(a) == RHA_symbol_t) && (ptype(b) == RHA_symbol_t))
    return UNWRAP_PTR(RHA_symbol_t, a) == UNWRAP_PTR(RHA_symbol_t, b);
  if ((ptype(a) == RHA_bool) && (ptype(b) == RHA_bool))
    return UNWRAP_BOOL(a) == UNWRAP_BOOL(b);
  if ((ptype(a) == RHA_int) && (ptype(b) == RHA_int))
    return UNWRAP_INT(a) == UNWRAP_INT(b);
  if ((ptype(a) == RHA_real_t) && (ptype(b) == RHA_int))
    return UNWRAP_REAL(a) == (real_t) UNWRAP_INT(b);
  if ((ptype(a) == RHA_int) && (ptype(b) == RHA_real_t))
    return (real_t) UNWRAP_INT(a) == UNWRAP_REAL(b);
  if ((ptype(a) == RHA_real_t) && (ptype(b) == RHA_real_t))
    return UNWRAP_REAL(a) == UNWRAP_REAL(b);
  else if ((ptype(a) == RHA_string_t) && (ptype(b) == RHA_string_t))
    return 0==strcmp(UNWRAP_PTR(RHA_string_t, a), UNWRAP_PTR(RHA_string_t, b));
  else
    return a == b;
}

bool notequal_fn(any_t a, any_t b)
{
  return !equalequal_fn(a, b);
}


any_t inc(any_t o)
{
  if (ptype(o) == RHA_int) {
    o->raw.i++;
    return o;
  }
  rha_error("'inc' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}

any_t dec(any_t o)
{
  if (ptype(o) == RHA_int) {
    o->raw.i--;
    return o;
  }
  rha_error("'dec' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}

any_t inc_copy(any_t o)
{
  if (ptype(o) == RHA_int) {
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
  if (ptype(o) == RHA_int) {
    any_t other = copy_pt(o);
    o->raw.i--;
    return other;
  }
  rha_error("'dec_copy' is currently only implemented for integers");
  assert(1==0);
  return 0; // make gcc happy
}


// there are two different 'checks'
// (i)  check whether 'x' is an instance of 't'
//      t.check(x) = {
//        return isparent(t.proto, x);
//      }  
// (ii) check whether 'x' is a subtype of 't'
//      t.isparent(x) = {
//        // go along the the parents of x to see whether we reach t
//      }

// 'check' looks along the parent hierarchy of the prototype
bool check(any_t t, any_t o)
{
  any_t proto = lookup(t, proto_sym);
  if (!proto)
    rha_error("(check) object %o doesn't have a slot 'proto' thus can "
	      "not have instances", t);
  return isparent(proto, o);
}

// 'isparent' looks along the parent hierarchy
bool isparent(any_t p, any_t o)
{
  do {
    if (addr(o) == addr(p))
      return true;
  } while ((o = lookup(o, parent_sym)));
  return false;
}


// 'pcheck' does a primitive type check which is sufficient (and much
// faster) for ptypes
bool pcheck(any_t t, any_t o)
{
  any_t proto = lookup(t, proto_sym);
  if (!proto)
    rha_error("(check) object %o doesn't have a slot 'proto'", t);
  return ptype(proto) == ptype(o);
}
