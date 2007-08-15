#include <assert.h>

#include "parse.h"
#include "rha_parser.h" // the link to BISON
#include "rha_types.h"
#include "object.h"
#include "symbol_fn.h"
#include "tuple_fn.h"
#include "eval.h"
#include "core.h"
#include "messages.h"
#include "utils.h"
#include "debug.h"

// let's require for now that all expressions must be separated by
// semicolon, otherwise we could make a list of keywords which are
// allowed to extend expression also after a 'curlied' expression

symbol_t semicolon_sym = 0;
symbol_t comma_sym = 0;
symbol_t dot_sym = 0;
symbol_t tuple_forced_sym = 0;

object_t prules = 0;  // the object containing all prules

void parse_init(object_t root, object_t module)
{
  // the following symbols are only used in 'parse.c'
  semicolon_sym = symbol_new(";");
  comma_sym = symbol_new(",");
  dot_sym = symbol_new(".");
  tuple_forced_sym = symbol_new("tuple_forced");

  // the object 'prules' is used to lookup the prules
  // its location should be changed here
  object_t modules = lookup(root, modules_sym);
  if (!modules) {
    fprint(stderr, "WARNING (parse_init):\n");
    fprint(stderr, "    Lookup of 'root.modules' failed.\n");
    fprint(stderr, "    We won't use any prules!\n");
  }
  prules = lookup(modules, symbol_new("prules"));
  if (!prules) {
    fprint(stderr, "WARNING (parse_init):\n");
    fprint(stderr, "    Lookup of 'root.modules.prules' failed.\n");
    fprint(stderr, "    We won't use any prules!\n");
    fprint(stderr, "    Try to change module order in 'rha_config.d'\n");
    prules = void_obj;
  }
}

////////////////////////////////////////////
// parsing from the outside to the inside //
////////////////////////////////////////////

object_t resolve(object_t env, object_t expr);
object_t resolve_list_by_head(object_t env, list_t source);
object_t resolve_list_by_prules(object_t env, list_t source);
object_t resolve_code_block(object_t env, list_t source);
object_t resolve_tuple(object_t env, list_t source);
object_t resolve_complex_literal(list_t source);
object_t resolve_macro(object_t env, tuple_t t);


object_t parse(object_t root, string_t s)
{
  // (0) run bison code
  list_t source = rhaparsestring(s);
  if (!source) return void_obj;

  // (1) resolve prules and macros creating a big function call
  return resolve_list_by_head(root, source);
}

object_t parse_file(object_t root, string_t fname)
{
  // (0) run bison code
  list_t source = rhaparsefile(fname);
  
  // (1) resolve prules and macros creating a big function call
  return resolve_list_by_head(root, source);
}




bool is_symbol(symbol_t a_symbol, object_t expr) {
  return (ptype(expr) == SYMBOL_T)
    && (UNWRAP_SYMBOL(expr) == a_symbol);
}


object_t resolve(object_t env, object_t expr)
{
  debug("resolve(%o, %o)\n", env, expr);
  if (ptype(expr) == LIST_T)
    return resolve_list_by_head(env, UNWRAP_PTR(LIST_T, expr));
  else
    return expr;
}

object_t resolve_list_by_head(object_t env, list_t source)
{
  debug("resolve_list_by_head(%o, %o)\n", env, WRAP_PTR(LIST_T, list_proto, source));
  // take the list 'source'
  // built a list 'sink'
  // finally return the 'sink' transformed into a tuple

  // The list 'source' is headed by the bracket type, which will
  // trigger different behavior
  assert(list_len(source) > 0);
  object_t l0 = list_first(source);
  symbol_t head;
  if ((ptype(l0) != SYMBOL_T)
      || (((head = UNWRAP_SYMBOL(l0)) != curlied_sym)
	  && (head != rounded_sym)
	  && (head != squared_sym)))
    return resolve_list_by_prules(env, source);

  // so we have a specially marked up list
  assert((head==rounded_sym)||(head==squared_sym)||(head==curlied_sym));
  // let's chop the head off
  list_popfirst(source);
  // split 'source' into sublist which are collected in 'sink'
  if (head==curlied_sym) {
    return resolve_code_block(env, source);
  }
  else if (head == rounded_sym) {
    return resolve_tuple(env, source);
  }
  else if (head == squared_sym) {
    return resolve_complex_literal(source);
  }
  else 
    // never reach this point
    assert(1==0);
}
 
object_t resolve_code_block(object_t env, list_t source)
{
  debug("resolve_code_block(%o, %o)\n", env, WRAP_PTR(LIST_T, list_proto, source));
  // code blocks  -->  returns a list_t
  // { x = 1; y = 7; deliver 5; { a=5; } x=5 }
  // split by semicolon, comma is not allowed
  if (list_len(source) == 0) {
    return WRAP_PTR(LIST_T, list_proto, list_new());
  }
  list_t sink = list_new();
  list_t part = list_new();
  object_t obj = 0;
  while ((obj = list_popfirst(source))) {
    if (is_symbol(semicolon_sym, obj)) {
      if (list_len(part)>0) {
	// split here and ignore the semicolon
	list_append(sink, resolve_list_by_prules(env, part));
	part = list_new();
      }
      continue;
    }
    else if (is_symbol(comma_sym, obj)) {
      // this is not allowed
      rha_error("(parsing) in a 'curlied' expression no comma allowed\n");
      assert(1==0);
    }
    else {
      list_append(part, obj);
    }
  }
  if (list_len(part)>0)
    list_append(sink, resolve_list_by_prules(env, part));
  return WRAP_PTR(LIST_T, list_proto, sink);  // a list!
}


object_t resolve_tuple(object_t env, list_t source)
{
  debug("resolve_tuple(%o, %o)\n", env, WRAP_PTR(LIST_T, list_proto, source));
  // grouped expression and argument lists
  // (x+1)*4 or (x, y, z) or (17,)
  // split only by comma, no semiclon is allowed
  if (list_len(source) == 0) {
    tuple_t t = tuple_new(1);
    tuple_set(t, 0, WRAP_SYMBOL(rounded_sym));
    return WRAP_PTR(TUPLE_T, tuple_proto, t);
  }
  list_t sink = list_new();
  list_t part = list_new();
  object_t obj = 0;
  bool last_was_comma = false;
  while ((obj = list_popfirst(source))) {
    if (is_symbol(comma_sym, obj)) {
      // split here and ignore the comma
      list_append(sink, resolve_list_by_prules(env, part));
      part = list_new();
      last_was_comma = true;
      continue;
    }
    else if (is_symbol(semicolon_sym, obj)) {
      // this is not allowed
      rha_error("(parsing) in a 'rounded' expression no semicolon allowed\n");
      assert(1==0);
    }
    list_append(part, obj);
    last_was_comma = false;
  }
  if (list_len(part) > 0)
    list_append(sink, resolve_list_by_prules(env, part));
  // three cases:
  // (1) stuff like: (17,42,)
  if (last_was_comma)
    if (list_len(sink)!=2) {
      assert(list_len(part) == 0);
      rha_error("(parsing) trailing comma only allowed for singleton tuple\n");
    }
    else {
      // (2) stuff like: (17,)
      list_prepend(sink, WRAP_SYMBOL(tuple_forced_sym));
    }
  else {
    // (3) stuff like: (17, 42)   (17)
    // comma wasn't the last symbol
    list_prepend(sink, WRAP_SYMBOL(rounded_sym));
  }
  return WRAP_PTR(TUPLE_T, tuple_proto, list_to_tuple(sink));
}


object_t resolve_complex_literal(list_t source)
{
  debug("resolve_complex_literal(%o)\n", WRAP_PTR(LIST_T, list_proto, source));
  // complex literals
  // [x, 17] or [ 2*x for x in [1,2,3]]
  
  // the idea here is that to define a complex literal
  // a type must define a list of separators, which determines the
  // order of subexpressions, and a constructor, which constructs
  // the object from the preprocessed list
  
  // problem:
  // * we need to call 'resolve' for each part
  // * we only can decide on the parts when we have splitted 
  
  // solution:
  // * we allow types to give a list of separators and types
  // * then there is some magic function (this one) which does the
  // work and tries all different literals as specified 
  // * this only works for default behavior.

  // for now: we delay the resolution of complex literals until 'eval'
  // time and create:
  //  [1,2,3] ==>  (literal_sym (tuple_sym [1,2,3]))
  tuple_t t1 = tuple_new(2);
  tuple_set(t1, 0, WRAP_SYMBOL(quote_sym));
  tuple_set(t1, 1, WRAP_PTR(LIST_T, list_proto, source));
  tuple_t t = tuple_new(2);
  tuple_set(t, 0, WRAP_SYMBOL(squared_sym));
  tuple_set(t, 1, WRAP_PTR(TUPLE_T, tuple_proto, t1));
  return WRAP_PTR(TUPLE_T, tuple_proto, source);
}

 
object_t resolve_list_by_prules(object_t env, list_t source)
{
  debug("resolve_prules(%o, %o)\n", env, WRAP_PTR(LIST_T, list_proto, source));
  // it is a tuple containing a white-spaced list
  // without any particular heading symbol
  
  // (1) find the prule with the lowest precendence
  // loop over all entries and check the symbols whether they are prules
  double best_priority = 0.0;  // initially the priority is like functions
  object_t best_prule  = 0;    // the object containing the best prule
  glist_iterator_t it;
  for (list_begin(source, &it); !list_done(&it); glist_next(&it)) {
    object_t symbol_obj = list_get(&it);
    if (ptype(symbol_obj) == SYMBOL_T) {
      object_t prule = lookup(prules, UNWRAP_SYMBOL(symbol_obj));
      if (!prule) continue;
      object_t prior = lookup(prule, priority_sym);
      if (prior && (ptype(prior) == REAL_T)) {
	double priority = UNWRAP_REAL(prior);
	// did we find a prule with a higher priority?
	if (priority > best_priority) {
	  // note that 'f' might not be a prule but since it has a
	  // priority it looks like one, then we will crash later
	  best_priority = priority;
	  best_prule = prule;
	}
      }
    }
  }
  // did we find a prule?
  if (best_priority > 0.0) {
    // resolve prule by constructing a call to that prule
    int tlen = 2;
    tuple_t prule_call = tuple_new(tlen);
    tuple_set(prule_call, 0, best_prule);
    tuple_set(prule_call, 1, WRAP_PTR(LIST_T, list_proto, source));
	                     // the list containing the parse tree
    // run the prule itself to resolve it
    object_t expr = call_fun(env, tlen, prule_call);
    if (ptype(expr) == TUPLE_T) {
      tuple_t t = UNWRAP_PTR(TUPLE_T, expr);
      // check the resulting tuple
      if ((tuple_len(t)==0) || (ptype(tuple_get(t, 0))!=SYMBOL_T))
	rha_error("(parsing) prule must create function call with function symbol\n");
      // resolve the arguments
      for (int i = 1; i<tuple_len(t); i++)
	tuple_set(t, i, resolve(env, tuple_get(t, i)));
      // finally resolve a possible macro that we got
      // note that even if the macro contains further macros in its
      // definition, we need to do resolve macros here only once,
      // since all other macros in the macro have already been
      // resolved when it was parsed
      expr = resolve_macro(env, t);
    }
    return expr;
  }
  else {
    // otherwise build function calls and deal with dots, which bind
    // less than function calls
    if (list_len(source) == 0)
      rha_error("(parsing) missing expression\n");
    object_t fncall = 0;
    object_t obj = 0;
    while ((obj = list_popfirst(source))) {
      
      if (ptype(obj) == LIST_T)
	obj = resolve(env, obj);
      if (!fncall) {
	// deal here with the singleton case
	if (ptype(obj) == TUPLE_T) {
	  tuple_t tt = UNWRAP_PTR(TUPLE_T, obj);
	  if (tuple_len(tt) == 2) {
	    object_t tt0 = tuple_get(tt, 0);
	    if (ptype(tt0) == SYMBOL_T) {
	      symbol_t tts = UNWRAP_SYMBOL(tt0);
	      if (tts != tuple_forced_sym) {
		obj = tuple_get(tt, 1);
	      }
	    }
	  }
	}
	if ((ptype(obj)==SYMBOL_T) && UNWRAP_SYMBOL(obj)==dot_sym)
	  rha_error("(parsing) something else than dot expected\n");
	fncall = obj;
	continue;
      }
      if ((ptype(obj) == SYMBOL_T) && UNWRAP_SYMBOL(obj)==dot_sym) {
	// ok we have a dotted expression
	// get the next object
	obj = list_popfirst(source);
	if (!obj || (ptype(obj) != SYMBOL_T))
	  rha_error("(parsing) a dot is always followed by a symbol\n");
	// make dotted expression
	tuple_t dot_rhs = tuple_new(2);
	tuple_set(dot_rhs, 0, WRAP_SYMBOL(quote_sym));
	tuple_set(dot_rhs, 1, obj);
	tuple_t t = tuple_new(3);
	// note that we use 'eval_sym' instead of 'lookup_sym', the
	// reason is that the later doesn't issue a 'lookup failed'
	// exception and returns NULL in that case, while 'eval' does
	// issue an exception and return 'void_obj'.
	tuple_set(t, 0, WRAP_SYMBOL(eval_sym));
	tuple_set(t, 1, fncall);
	tuple_set(t, 2, WRAP_PTR(TUPLE_T, tuple_proto, dot_rhs));
	fncall = WRAP_PTR(TUPLE_T, tuple_proto, t);
	continue;
      }
      if (ptype(obj) != TUPLE_T)
	rha_error("(parsing) argument list expected (1)\n");
      tuple_t t = UNWRAP_PTR(TUPLE_T, obj);
      object_t t0 = tuple_get(t, 0);
      if (ptype(t0) != SYMBOL_T)
	rha_error("(parsing) argument list expected (2)\n");
      symbol_t s = UNWRAP_SYMBOL(t0);
      if (s != rounded_sym)
	rha_error("(parsing) argument list expected (3)\n");
      // replace the rounded_sym with the function call so far
      tuple_set(t, 0, fncall);
      fncall = resolve_macro(env, t);
    }
    return fncall;
  }
  assert(1==0);  // never reach this point
}


object_t replace_expr(object_t expr, symbol_t s, object_t sub)
{
  // replace all occurences of 's' in 'expr' by 'sub'
  if (ptype(expr) == SYMBOL_T) {
    symbol_t sy = UNWRAP_SYMBOL(expr);
    if (sy == s)
      return sub;
  }
  else if (ptype(expr) == TUPLE_T) {
    tuple_t t = UNWRAP_PTR(TUPLE_T, expr);
    // create a new tuple
    int tlen = tuple_len(t);
    tuple_t newt = tuple_new(tlen);
    for (int i=0; i<tuple_len(t); i++)
      tuple_set(newt, i, replace_expr(tuple_get(t, i), s, sub));
    return WRAP_PTR(TUPLE_T, tuple_proto, newt);
  }
  return expr;
}


object_t resolve_macro(object_t env, tuple_t t)
{
  int tlen = tuple_len(t);
  if (tlen == 0) 
    return WRAP_PTR(TUPLE_T, tuple_proto, t);
  object_t t0 = tuple_get(t, 0);
  if (ptype(t0) != SYMBOL_T)
    return WRAP_PTR(TUPLE_T, tuple_proto, t);
  symbol_t s = UNWRAP_SYMBOL(t0);
  object_t macro = lookup(env, s);
  if (!macro)
    return WRAP_PTR(TUPLE_T, tuple_proto, t);
  if (!lookup(macro, symbol_new("ismacro")))
    return WRAP_PTR(TUPLE_T, tuple_proto, t);
  object_t macro_body = lookup(macro, symbol_new("fnbody"));
  if (!macro_body)
    return WRAP_PTR(TUPLE_T, tuple_proto, t);
  object_t _macro_args = lookup(macro, symbol_new("argnames"));
  if (!_macro_args)
    rha_error("(parsing) macro found but args are missing");
  tuple_t macro_args = UNWRAP_PTR(TUPLE_T, _macro_args);
  if (tuple_len(macro_args) != tlen-1)
    rha_error("(parsing) wrong macro arc number");
  for (int i=1; i<tlen; i++) {
    // go through the 'macro_body' and replace all occurences of
    // macro_args
    macro_body = replace_expr(macro_body, 
			      UNWRAP_SYMBOL(tuple_get(macro_args, i-1)),
			      tuple_get(t, i));
  }
  return macro_body;
}
