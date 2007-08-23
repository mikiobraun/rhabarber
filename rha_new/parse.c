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
#include "excp.h"

// let's require for now that all expressions must be separated by
// semicolon, otherwise we could make a list of keywords which are
// allowed to extend expression also after a 'curlied' expression

object_t prule_failed_excp = 0; // exception

object_t prules = 0;  // the object containing all prules

symbol_t semicolon_sym = 0;
symbol_t comma_sym = 0;
symbol_t dot_sym = 0;

void parse_init(object_t root, object_t module)
{
  prule_failed_excp = excp_new("prule failed");

  semicolon_sym = symbol_new(";");
  comma_sym = symbol_new(",");
  dot_sym = symbol_new(".");

  // the object 'prules' is used to lookup the prules
  // its location should be changed here
  object_t modules = lookup(root, modules_sym);
  if (!modules) {
    fprint(stderr, "WARNING (parse_init):\n");
    fprint(stderr, "    Lookup of 'root.modules' failed.\n");
    fprint(stderr, "    We won't use any prules!\n");
  }
  prules = lookup(modules, prules_sym);
  if (!prules) {
    fprint(stderr, "WARNING (parse_init):\n");
    fprint(stderr, "    Lookup of 'root.modules.prules' failed.\n");
    fprint(stderr, "    We won't use any prules!\n");
    fprint(stderr, "    Try to change module order in 'rha_config.d'\n");
    prules = 0;
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
object_t resolve_complex_literal(object_t env, list_t source);
object_t resolve_macro(object_t env, tuple_t t);


object_t parse(object_t root, string_t s)
{
  // (0) run bison code
  list_t source = rhaparsestring(s);
  if (list_len(source)==0) return 0;

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
  if (!expr) return 0;
  // note that tuples are not resolved anymore
  // thus: if prules resolve the arguments themselves, they become
  // tuples and that we it doesn't matter if we call 'resolve' again
  // on them
  //debug("resolve(%o, %o)\n", env, expr);
  if (ptype(expr) == LIST_T)
    return resolve_list_by_head(env, UNWRAP_PTR(LIST_T, expr));
  else
    return expr;
}

object_t resolve_list_by_head(object_t env, list_t source)
{
  //debug("resolve_list_by_head(%o, %o)\n", env, WRAP_PTR(LIST_T, source));
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
    return resolve_complex_literal(env, source);
  }
  else {
    // never reach this point
    assert(1==0);
    return 0; // make gcc happy
  }
}
 
object_t resolve_code_block(object_t env, list_t source)
{
  //debug("resolve_code_block(%o, %o)\n", env, WRAP_PTR(LIST_T, source));
  // code blocks  -->  returns a tuple_t
  // { x = 1; y = 7; deliver 5; { a=5; } x=5 }
  // split by semicolon, comma is not allowed
  if (list_len(source) == 0) {
    return WRAP_PTR(TUPLE_T, tuple_new(0));
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
      rha_error("(parsing) in a 'curlied' expression no comma allowed");
      assert(1==0);
    }
    else {
      list_append(part, obj);
    }
  }
  if (list_len(part)>0)
    list_append(sink, resolve_list_by_prules(env, part));
  list_prepend(sink, WRAP_SYMBOL(do_sym));
  return WRAP_PTR(TUPLE_T, list_to_tuple(sink));  // a list!
}


object_t resolve_tuple(object_t env, list_t source)
{
  //debug("resolve_tuple(%o, %o)\n", env, WRAP_PTR(LIST_T, source));
  // grouped expression and argument lists
  // (x+1)*4 or (x, y, z)
  // split only by comma, no semiclon is allowed
  if (list_len(source) == 0) {
    tuple_t t = tuple_new(1);
    tuple_set(t, 0, WRAP_SYMBOL(rounded_sym));
    return WRAP_PTR(TUPLE_T, t);
  }
  list_t sink = list_new();
  list_t part = list_new();
  object_t obj = 0;
  while ((obj = list_popfirst(source))) {
    if (is_symbol(comma_sym, obj)) {
      if (list_len(source)==0)
	rha_error("(parsing) trailing comma not allowed");
      // split here and ignore the comma
      switch (list_len(part)) {
      case 0: rha_error("can't start tuple with COMMA");
      case 1: list_append(sink, resolve(env, list_first(part))); break;
      default:
	list_append(sink, resolve_list_by_prules(env, part));
      }
      part = list_new();
      continue;
    }
    else if (is_symbol(semicolon_sym, obj)) {
      // this is not allowed
      rha_error("(parsing) in a 'rounded' expression no semicolon allowed");
      assert(1==0);
    }
    list_append(part, obj);
  }
  if (list_len(part) > 0)
    list_append(sink, resolve_list_by_prules(env, part));
  list_prepend(sink, WRAP_SYMBOL(rounded_sym));
  //debug("return (resolve_tuple): %o\n", WRAP_PTR(TUPLE_T, list_to_tuple(sink)));
  return WRAP_PTR(TUPLE_T, list_to_tuple(sink));
}


object_t resolve_complex_literal(object_t env, list_t source)
{
  //debug("resolve_complex_literal(%o)\n", WRAP_PTR(LIST_T, source));
  // complex literals
  // [x, 17] or [ 2*x for x in [1,2,3]]
  
  // the idea here is that to define a complex literal
  // a type must define a list of separators, which determines the
  // order of subexpressions, and a constructor, which constructs
  // the object from the preprocessed list
  
  // problem:
  // * we need to call 'resolve' for each part
  // * we only can decide on the parts after we have splitted 
  
  // solution:
  // * we allow types to give a list of separators and types

  // we delay the resolution of the subexpression until either the
  // default literal resolver is called or a special one
  //  [1,2,3] ==>  (literal_sym (tuple_sym [1,2,3]))
  tuple_t t = tuple_new(3);
  tuple_set(t, 0, WRAP_SYMBOL(literal_sym));
  tuple_set(t, 1, env);
  tuple_set(t, 2, quoted(WRAP_PTR(LIST_T, source)));
  return WRAP_PTR(TUPLE_T, t);
}

 
object_t find_best_prule(object_t env, list_t source)
{
  // find the prule with the lowest precendence
  // loop over all entries and check the symbols whether they are prules
  double best_priority = 0.0;  // initially the priority is like functions
  object_t best_prule  = 0;    // the object containing the best prule
  list_it_t it;
  for (it = list_begin(source); !list_done(it); glist_next(it)) {
    object_t symbol_obj = list_get(it);
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
  if (best_priority > 0.0)
    return best_prule;
  else
    return 0;
}

void resolve_args(object_t env, tuple_t t)
{
  // resolve the arguments
  int tlen = tuple_len(t);
  for (int i = 1; i<tlen; i++)
    tuple_set(t, i, resolve(env, tuple_get(t, i)));
}

object_t resolve_prule(object_t env, list_t source, object_t prule)
{
  // resolve prule by constructing a call to that prule
  int tlen = 3;
  tuple_t prule_call = tuple_new(tlen);
  tuple_set(prule_call, 0, prule);
  tuple_set(prule_call, 1, env);
  tuple_set(prule_call, 2, WRAP_PTR(LIST_T, source));

  // the list containing the parse tree
  // run the prule itself to resolve it
  object_t expr = call_fun(env, tlen, prule_call);

  // check what we got
  if (ptype(expr) == TUPLE_T) {
    tuple_t t = UNWRAP_PTR(TUPLE_T, expr);
    // check the resulting tuple
    if ((tuple_len(t)==0) || (ptype(tuple_get(t, 0))!=SYMBOL_T))
      rha_error("(parsing) prule must create function call with function symbol");

    // note that we can always resolve the arguments, since they are
    // only resolve if they are lists (see resolve()), thus if a prule
    // does resolve the args itself, they become tuples are fixed then...
    resolve_args(env, t);
    
    // finally resolve a possible macro that we got
    // note that even if the macro contains further macros in its
    // definition, we need to do resolve macros here only once,
    // since all other macros in the macro have already been
    // resolved when it was parsed
    expr = resolve_macro(env, t);
  }
  //debug("return (prules) %o\n", expr);
  return expr;
}

object_t remove_grouping_brackets(object_t obj)
{
  // if 'obj' is a tuple starting with 'rounded_sym' it must only
  // be a singleton.  in that case the rounded brackets are removed
  if (ptype(obj) == TUPLE_T) {
    tuple_t tt = UNWRAP_PTR(TUPLE_T, obj);
    if (tuple_len(tt) > 0) {
      object_t tt0 = tuple_get(tt, 0);
      if (ptype(tt0) == SYMBOL_T) {
	symbol_t tts = UNWRAP_SYMBOL(tt0);
	if (tts == rounded_sym) {
	  if (tuple_len(tt) != 2)
	    rha_error("(parsing) rounded brackets are only for "
		      "grouping or arg lists");
	  obj = tuple_get(tt, 1);
	}
      }
    }
  }
  return obj;
}

bool_t is_rounded_tuple(object_t obj)
{
  if (ptype(obj) != TUPLE_T)
    return false;
  tuple_t t = UNWRAP_PTR(TUPLE_T, obj);
  if (tuple_len(t) == 0)
    return false;
  object_t t0 = tuple_get(t, 0);
  if (ptype(t0) != SYMBOL_T)
    return false;
  symbol_t s = UNWRAP_SYMBOL(t0);
  if (s != rounded_sym)
    return false;
  // ok, it is a roudned tupled
  return true;
}


object_t resolve_dots_and_fn_calls(object_t env, list_t source)
{
  // what this function does:
  //   source        result
  //   ----------------------------------------------
  //   a             a
  //   a.x           (eval a \x)
  //   a.x.y         (eval (eval a \x) \y)
  //   a(z)          (a z)
  //   a(z)(zz)      ((a z) zz)
  //   a.x(z)        (callslot a \x z)
  //   a.x.y(z)      (callslot (eval a \x) \y z)
  //   a.x(z)(zz)    ((callslot a \x z) zz)
  //   a.x.y(z)(zz)  ((callslot (eval a \x) \y z) zz)
  
  // ok there is more:
  //   list().append(17);
  // more precisely:
  //   (1) the first thing can be anything but a 'dot'
  //   (2) a series of dots and argument lists

  if (list_len(source) == 0)
    rha_error("(parsing) missing expression");

  // (1.1) get the first piece and resolve it
  object_t obj = resolve(env, list_popfirst(source));

  // (1.2) remove rounded brackets for grouping
  obj = remove_grouping_brackets(obj);

  // (1.3) check that we don't start with the 'dot'
  if ((ptype(obj)==SYMBOL_T) && UNWRAP_SYMBOL(obj)==dot_sym)
    rha_error("(parsing) something else than dot expected");

  // (1.4) start the expression
  object_t expr = obj;

  // we need to keep a flag which tells us later whether we have the
  // usual function call or whether the last dot results in a
  // slotcall. 
  bool_t dotted = false;
  while ((obj = resolve(env, list_popfirst(source)))) {
    if ((ptype(obj) == SYMBOL_T) && UNWRAP_SYMBOL(obj)==dot_sym) {
      // (2) dotted expression

      // get the next object as well which must be a symbol
      obj = list_popfirst(source);
      if (!obj || (ptype(obj) != SYMBOL_T))
	rha_error("(parsing) a dot is always followed by a symbol");

      // make dotted expression
      tuple_t t = tuple_new(3);
      // note that we use 'eval_sym' instead of 'lookup_sym', the
      // reason is that the later doesn't issue a 'lookup failed'
      // exception and returns 'void' in that case, while 'eval' does
      // issue an exception and return 'void' (==0).
      tuple_set(t, 0, WRAP_SYMBOL(eval_sym));
      tuple_set(t, 1, expr);
      tuple_set(t, 2, quoted(obj));
      
      // we are potentially constructing a method call
      dotted = true;
      expr = WRAP_PTR(TUPLE_T, t);
      continue;
    }
    else if (is_rounded_tuple(obj)) {
      // (3) function/method call
      if (dotted) {
	// construct a method call
	dotted = false; // wait for the next dot
	// for a.f(x,y) we have by now two ingredients:
	// (i)  expr == (eval a \f)
	// (ii) obj  == (rounded x y)
	// from those we construct
	//      (callslot a \f x y)
	list_t l = tuple_to_list(UNWRAP_PTR(TUPLE_T, obj));
	tuple_t t = UNWRAP_PTR(TUPLE_T, expr);
	list_popfirst(l);                           // remove 'rounded'
	list_prepend(l, tuple_get(t, 2));           // prepend '\f'
	list_prepend(l, tuple_get(t, 1));           // prepend 'a'
	list_prepend(l, WRAP_SYMBOL(callslot_sym)); // prepend 'callslot'
	expr = WRAP_PTR(TUPLE_T, list_to_tuple(l));
	continue;
      }
      else {
	// construct a plain function call
	// replace the rounded_sym with the expression so far
	tuple_t t = UNWRAP_PTR(TUPLE_T, obj);
	tuple_set(t, 0, expr);
	expr = resolve_macro(env, t);
	continue;
      }
    }
    else {
      rha_error("(parsing) argument list or dot expected, found \"%o\"", obj);
    }
  }

  //debug("return (resolve_dots_and_funcalls) %o\n", expr);
  return expr;
}


object_t resolve_list_by_prules(object_t env, list_t source)
{
  //debug("resolve_prules(%o, %o)\n", env, WRAP_PTR(LIST_T, source));
  // it is a tuple containing a white-spaced list
  // without any particular heading symbol
  
  object_t prule = find_best_prule(env, source);
  object_t excp = 0;
  object_t result = 0;
  if (prule) {
    // try to call that prule
    try {
      // don't call 'return' inside 'try-catch' blocks!!!
      result = resolve_prule(env, source, prule);
    }
    catch (excp) {
      if (excp == prule_failed_excp) {
	// try it again
	// hopefully, the prule has adjusted the source to avoid it
	// from being called again
	// this happens (e.g.) for 'minus_pr' if it encounters a prefix-minus
	result = resolve_list_by_prules(env, source);
      }
      else {
	// pass the unknown exception on
	throw(excp);
      }
    }
    return result;
  }
  else
    return resolve_dots_and_fn_calls(env, source);
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
    return WRAP_PTR(TUPLE_T, newt);
  }
  return expr;
}


object_t resolve_macro(object_t env, tuple_t t)
{
  int tlen = tuple_len(t);
  if (tlen == 0) 
    return WRAP_PTR(TUPLE_T, t);
  object_t t0 = tuple_get(t, 0);
  if (ptype(t0) != SYMBOL_T)
    return WRAP_PTR(TUPLE_T, t);
  symbol_t s = UNWRAP_SYMBOL(t0);
  object_t macro = lookup(env, s);
  if (!macro)
    return WRAP_PTR(TUPLE_T, t);
  if (!lookup(macro, ismacro_sym))
    return WRAP_PTR(TUPLE_T, t);
  object_t macro_body = lookup(macro, fnbody_sym);
  if (!macro_body)
    return WRAP_PTR(TUPLE_T, t);
  object_t _macro_args = lookup(macro, argnames_sym);
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
