#include <assert.h>

#include "parse.h"
#include "rha_parser.h" // the link to BISON
#include "rha_types.h"
#include "object.h"
#include "symbol_fn.h"
#include "tuple_fn.h"
#include "eval.h"
#include "messages.h"

//----------------------------------------------------------------------
// helper functions for the parsing

object_t split(list_t source, symbol_t head) {
  list_t sink = list_new();
  while (list_len(source) > 0) {
    object_t next = list_popfirst(source);
    if (ptype(next) == SYMBOL_T) {
      symbol_t s = UNWRAP_SYMBOL(next);
      if ((head==curlied_sym && s==symbol_new(";"))
	  || (head==rounded_sym && s==symbol_new(",")))
	continue;
    }
    list_append(sink, next);
  }
  return WRAP_PTR(TUPLE_T, tuple_proto, list_to_tuple(sink));
}

bool is_rounded(object_t item) {
  object_t lfn = 0;
  return (ptype(item) == LIST_T)
    && (ptype(lfn=list_first(UNWRAP_PTR(LIST_T, item))) == SYMBOL_T)
    && (UNWRAP_SYMBOL(lfn) == rounded_sym);
}

bool is_args(object_t item) {
  tuple_t up = 0;
  object_t lfn = 0;
  return (ptype(item) == TUPLE_T)
    && (tuple_len(up=UNWRAP_PTR(TUPLE_T, item)) > 0)
    && (ptype(lfn=tuple_get(up, 0)) == SYMBOL_T)
    && (UNWRAP_SYMBOL(lfn) == rounded_sym);
}

bool is_grouped(object_t item) {
  if (ptype(item) != TUPLE_T)
    return false;
  tuple_t t = UNWRAP_PTR(TUPLE_T, item);
  assert(tuple_len(t)>0);
  if (UNWRAP_SYMBOL(tuple_get(t, 0)) != rounded_sym)
    return false;
  if (tuple_len(t) > 2)
    rha_error("parse error, not a comma separated rounded tuple expected");
  return true;
}

//

#define OPSTACK_SIZE 256
static struct
{
  object_t prule;
  real_t precedence;
} opstack[OPSTACK_SIZE];
static int ops = -1;

// push an operator on the stack
static void opstack_push(object_t prule, real_t precedence)
{
  ops++; assert(ops < OPSTACK_SIZE);
  opstack[ops].prule = prule;
  opstack[ops].precedence = precedence;
}

// pop an operator from the stack
static void opstack_pop()
{
  ops--; assert(ops >= 0);
}

// get the tos
static object_t tos_prule()
{
  return opstack[ops].prule;
}

static real_t tos_precedence()
{
  return opstack[ops].precedence;
}

// actually call the prule with the arguments on the sink and the right arity
static object_t call_prule(object_t env, list_t sink, object_t prule)
{
  int arity = UNWRAP_INT(lookup(prule, symbol_new("arity")));

  // construct the function call
  tuple_t fncall = tuple_new(arity + 1);
  tuple_set(fncall, 0, lookup(prule, symbol_new("code")));
  for(int i = 0; i < arity; i++)
    tuple_set(fncall, i+1, list_popfirst(sink));

  return eval(env, WRAP_PTR(TUPLE_T, tuple_proto, fncall));
}

// now, the next functions actually "do something" ;)
static void opstack_resolve_till(object_t env, list_t sink, real_t precedence)
{
  while(tos_precedence() > precedence) {
    list_prepend(sink, call_prule(env, sink, tos_prule()));
    opstack_pop();
  }
}

//----------------------------------------------------------------------

object_t resolve(object_t root, object_t source);
object_t parse(object_t root, string_t s)
{
  // (0) run bison code
  list_t source = rhaparsestring(s);
  
  // (1) resolve function calls and prules
  return resolve(root, WRAP_PTR(LIST_T, list_proto, source));
}

object_t resolve(object_t root, object_t obj) 
{
  // (-1) do we have a non-list?
  if (ptype(obj)!=LIST_T)
    return obj;
  // we have list!
  list_t source = UNWRAP_PTR(LIST_T, obj);

  // (0) pop off the head, which is in (3) used
  // to decide how to split
  symbol_t head = UNWRAP_SYMBOL(list_popfirst(source));
  assert((head == rounded_sym)
	 || (head == curlied_sym)
	 || (head == squared_sym));

  // (1) resolve function calls
  // go from left to right
  // if there is a keyword, ignore depending on its arity
  // otherwise built function call
  object_t keywords = lookup(root, symbol_new("keywords"));
  object_t prules = lookup(root, symbol_new("prules"));

  if(!prules) {
    list_prepend(source, WRAP_SYMBOL(head));
    return WRAP_PTR(LIST_T, list_proto, source);
  }

  list_t sink = list_new();
  int ignore = 0;
  object_t lfn = 0;
  object_t fncall = 0;
  while (list_len(source)>0) {
    object_t next = list_popfirst(source);
    if (ignore-- > 0) {
      // here can't be any pending 'fncall'
      assert(fncall==0);

      // 'next' can be ignored for function calls 
      // but it must be 'grouped', i.e. a rounded singleton
      object_t next_resolved = resolve(root, next);
      if (!is_grouped(next_resolved))
	rha_error("parse error, rounded singleton");
      list_append(sink, next_resolved);
    }
    else if (ptype(next)==SYMBOL_T) {
      // is it a keyword?
      symbol_t token = UNWRAP_SYMBOL(next);
      object_t keyword = lookup(keywords, token);
      if (keyword) {
	// yes, it is a keyword, thus we can ignore that expressions
	if (ignore > 0) 
	  rha_error("parse error, not yet a keyword expected");
	ignore = UNWRAP_INT(lookup(keyword, symbol_new("arity")));
	ignore--;  // ignore == arity-1
	// is there a pending 'fncall'?
	if (fncall) {
	  list_append(sink, fncall);
	  fncall = 0;
	}
	list_append(sink, next);
      }
      else if (lookup(prules, token)) {
	// another operator found, thus:
	if (fncall) {
	  list_append(sink, fncall);
	  fncall = 0;
	}
	list_append(sink, next);
      }
      else {
	// no, it is not a keyword and not an operator, 
	// but still it is a symbol
	// so we need to start a new 'fncall'
	if (!fncall)
	  rha_error("parse error, not another symbol expected");
	fncall = next;
      }
    }
    else if (ptype(next) != LIST_T) {
      // literals
      list_append(sink, next);
    }
    else {
      // now we can assume it is a list_t, otherwise the parser did
      // something wrong, this is checked in the next call
      object_t next_resolved = resolve(root, next);

      // did we already start to built the next function call?
      if (fncall) {
	// let's extend the fncall, which requires an 'fnargs' expressions
	if (!is_args(next_resolved))
	  rha_error("parse error, expecting a rounded expression for function call");
	
	// prepend the functioncall-so-far
	tuple_set(UNWRAP_PTR(TUPLE_T, next_resolved), 0, fncall);
	fncall = next_resolved;
      }
      else {
	// we start over, so it can be anything
	// however, if it is a rounded singleton, we need to remove
	// the brackets
	if (is_grouped(next_resolved))
	  next_resolved = tuple_get(UNWRAP_PTR(TUPLE_T, next_resolved), 1);
	fncall = next_resolved;
      }
    }
  }
  
  if (fncall) {
    list_append(sink, fncall);
  }

  source = sink;
  
  // (2) resolve prule from inside out
  // go from right to left
  // use a stack and some magic

  sink = list_new();

  while(list_len(source) > 0) {
    object_t o = list_poplast(source);
    object_t k, p;

    // is the symbol an operator or a keyword?
    if (ptype(o) == SYMBOL_T) {
      if ( p = lookup(prules, UNWRAP_SYMBOL(o)) ) {
	int_t style = UNWRAP_INT(lookup(p, symbol_new("style")));
	real_t prec = UNWRAP_INT(lookup(p, symbol_new("precedence")));
	
	switch (style) {
	case POSTFIX: 
	  // no competition with other operators, just push prule on the stack
	  opstack_push(p, prec);
	  break;
	case INFIX:
	  // compete for other operators, push ourselves on the stack
	  if (prec < tos_precedence())
	    opstack_resolve_till(root, sink, prec);
	  opstack_push(p, prec);
	  break;
	case PREFIX: 
	case FREEFIX:
	  // take care of the others, then directly apply the prule
	  if (prec < tos_precedence())
	    opstack_resolve_till(root, sink, prec);
	  list_prepend(sink, call_prule(root, sink, p));
	  break;
	}	
      } // end if "it is a prule"
    } 
    else {
      list_prepend(sink, o);
    }
  }

  // clean up at the end
  opstack_resolve_till(root, sink, -1.0);

  // (3) split depending on the initial head
  return split(sink, head);
}

object_t keyword(string_t s, int_t a) {
  object_t o = new();
  assign(o, symbol_new("token"), WRAP_SYMBOL(symbol_new(s)));
  assign(o, symbol_new("arity"), WRAP_INT(a));
  return o;
}


#define MAKE_PRULE(ttt)\
  object_t o = new();\
  assign(o, symbol_new("token"), WRAP_SYMBOL(symbol_new(s)));\
  assign(o, symbol_new("code"), f);\
  assign(o, symbol_new("precedence"), WRAP_REAL(ttt ## PREC));\
  assign(o, symbol_new("style"), WRAP_INT(ttt ## FIX));

object_t macrofix_prule(string_t s, object_t f) {
  MAKE_PRULE(MACRO);
  return o;
}


object_t postfix_prule(string_t s, object_t f) {
  MAKE_PRULE(POST);
  assign(o, symbol_new("arity"), WRAP_INT(1));
  return o;
}


object_t prefix_prule(string_t s, object_t f) {
  MAKE_PRULE(PRE);
  assign(o, symbol_new("arity"), WRAP_INT(1));
  return o;
}


object_t infix_prule(string_t s, object_t f, real_t prec, int_t bind) {
  object_t o = new();
  assign(o, symbol_new("token"), WRAP_SYMBOL(symbol_new(s)));
  assign(o, symbol_new("precedence"), WRAP_REAL(prec));
  assign(o, symbol_new("style"), WRAP_INT(INFIX));
  assign(o, symbol_new("bind"), WRAP_INT(bind));
  assign(o, symbol_new("arity"), WRAP_INT(2));
  return o;
}


object_t freefix_prule(string_t s, object_t f, tuple_t parts) {
  MAKE_PRULE(FREE);
  assign(o, symbol_new("parts"), WRAP_PTR(TUPLE_T, tuple_proto, parts));

  // compute total arity
  int_t arity = 0;
  for(int i = 0; i < tuple_len(parts); i++)
    arity += UNWRAP_INT(lookup(tuple_get(parts, i), symbol_new("arity"))) + 1;
  assign(o, symbol_new("arity"), WRAP_INT(arity));

  return o;
}

