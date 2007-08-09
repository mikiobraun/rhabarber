#include "parse.h"
#include "rha_parser.h" // the link to BISON

list_t split(list_t source, symbol_t) {
  rha_error("note yet");
}


bool is_rounded(object_t item) {
  object_t lfn = 0;
  return (ptype(item) == LIST_T)
    && (ptype(lfn=list_first(item)) == SYMBOL_T)
    && (UNWRAP_SYMBOL(lfn) == rounded_sym);
}

bool is_args(object_t item) {
  tuple_t up = 0;
  object_t lfs = 0;
  return (ptype(item) == TUPLE_T)
    && (tuple_len(up=UNWRAP_PTR(TUPLE_T, item)) > 0)
    && (ptype(lfn=tuple_get(up, 0)) == SYMBOL_T)
    && (UNWRAP_SYMBOL(lfn) == args_sym);
}

object_t parse(object_t root, string_t s) {

  // (0) run bison code
  list_t source = rhaparsestring(s);
  
  // (1) resolve function calls and prules
  return resolve(source);
}

object_t resolve(list_t source) {

  // (0) pop off the head, which is in (3) used
  // to decide how to split
  symbol_t head = list_popfirst(source);
  assert((head == rounded_sym)
	 || (head == curlied_sym)
	 || (head == squared_sym));

  // (1) resolve function calls
  // go from left to right
  // if there is a keyword, ignore depending on its arity
  // otherwise built function call
  object_t keywords = lookup(root, symbol_new("keywords"));
  list_t sink = list_new();
  int ignore = 0;
  object_t lfn = 0;
  object_t fncall = 0;
  while (list_len(source)>0) {
    object_t next = list_popfirst(l);
    if (ignore-- > 0) {
      // here can't be any pending 'fncall'
      assert(fncall==0);

      // 'next' can be ignored for function calls 
      // but it must be 'rounded'
      if (!is_rounded(next))
	rha_error("parse error, rounded expr expected");
      list_append(sink, resolve(next));
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
	  list_append(fncall);
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
    else {
      // now we can assume it is a list_t, otherwise the parser did
      // something wrong, this is checked in the next call
      object_t next_resolved = resolve(next);

      // did we already start to built the next function call?
      if (fncall) {
	// let's extend the fncall, which requires an 'fnargs' expressions
	if (!is_args(next_resolved))
	  rha_error("parse error, expecting a rounded expression for function call");
	
	// prepend the functioncall-so-far
        list_popfirst(next_resolved);
	list_prepend(next_resolved, fncall);
      }
      else {
	// we start over, so it can be anything
	fncall = next_resolved;
      }
    }
  }
  source = sink;
  
  // (2) resolve prule from inside out
  // go from right to left
  // use a stack and some magic


  // (3) split depending on the initial head
  switch (head) {
  case curlied_sym: 
    source = split(source, symbol_new(';')); 
    break;
  case rounded_sym: 
    source = split(source, symbol_new(',')); 
    break;
  case squared_sym: 
    // don't split!
    rha_error("squared brackets not yet implemented");
    break;
  otherwise:
    assert(false);
  }
}

object_t add_keyword(string_t s, int_t a) {
  object_t o = new();
  assign(o, symbol_new("token"), symbol_new(s));
  assign(o, symbol_new("arity"), a);
  return o;
}

#define MACROFIX 0
#define POSTFIX  1
#define PREFIX   2
#define INFIX    3
#define FREEFIX  4

#define MACROPREC   1.0
#define POSTPREC    2.0
#define PREPREC     3.0
// INFIX should be between 3.0 and 5.0 for numerical infix operators
#define FREEPREC    5.0
// INFIX should be larger than 5.0 for assignments
// INFIX for comma, semicolon even larger

#define MAKE_PRULE(ttt)\
  object_t o = new();\
  assign(o, symbol_new("token"), symbol_new(s));\
  assign(o, symbol_new("code"), f);\
  assign(o, symbol_new("precedence"), ttt ## PREC);\
  assign(o, symbol_new("style"), ttt ## FIX);

object_t add_macrofix_prule(string_t s, object_t f) {
  MAKE_PRULE(MACRO);
  return o;
}


object_t add_postfix_prule(string_t s, object_t f) {
  MAKE_PRULE(POST);
  return o;
}


object_t add_prefix_prule(string_t s, object_t f) {
  MAKE_PRULE(PRE);
  return o;
}


object_t add_infix_prule(string_t s, object_t f, double_t prec, int_t bind) {
  object_t o = new();
  assign(o, symbol_new("token"), symbol_new(s));
  assign(o, symbol_new("precedence"), prec);
  assign(o, symbol_new("style"), INFIX);
  assign(o, symbol_new("bind"), bind);
  return o;
}


object_t add_freefix_prule(string_t s, object_t f, tuple_t parts) {
  MAKE_PRULE(FREE);
  assign(o, symbol_new("parts"), parts);
  return o;
}

