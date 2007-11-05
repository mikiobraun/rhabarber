#include <assert.h>
#include <math.h>

#include "parse.h"
#include "rha_parser_fcts.h" // the link to BISON
#include "rha_types.h"
#include "object.h"
#include "symbol_fn.h"
#include "tuple_fn.h"
#include "eval.h"
#include "core.h"
#include "messages.h"
#include "utils.h"
#include "debug.h"
#include "prules.h"
#include "excp.h"

// let's require for now that all expressions must be separated by
// semicolon, otherwise we could make a list of keywords which are
// allowed to extend expression also after a 'curlied' expression

any_t prule_failed_excp = 0; // exception

any_t prules = 0;  // the object containing all prules

symbol_t dot_sym = 0;
symbol_t ellipsis_sym = 0;  // ...

void parse_init(any_t root, any_t module)
{
  prule_failed_excp = excp_new("prule failed");

  dot_sym = symbol_new(".");
  ellipsis_sym = symbol_new("...");

  // the object 'prules' is used to lookup the prules
  // its location should be changed here
  any_t modules = lookup(root, modules_sym);
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

any_t resolve(any_t env, list_t source);
any_t resolve_macro(any_t env, tuple_t t);
any_t resolve_dots_and_fn_calls(any_t env, list_t source);
any_t resolve_patterns(any_t env, list_t source);


// auxillary functions
static any_t call_prule(any_t env, list_t source, any_t prule);
static any_t find_best_prule(any_t env, list_t source);
static double get_priority(any_t prule);
static any_t replace_expr(any_t expr, symbol_t s, any_t sub);


//----------------------------------------------------------------------
// resolving prules
//
// the main workhorse for resolving prules

// find the best prule, and apply the prule
any_t resolve(any_t env, list_t source)
{
  //debug("resolve(%o, %o)\n", env, WRAP_PTR(RHA_list_t, source));
  any_t prule = find_best_prule(env, source);
  any_t excp = 0;
  any_t result = 0;
  if (prule) {
    // try to call that prule
    try {
      // don't call 'return' inside 'try-catch' blocks!!!
      result = call_prule(env, source, prule);
    }
    catch (excp) {
      if (excp == prule_failed_excp) {
	// try it again
	// hopefully, the prule has adjusted the source to avoid it
	// from being called again
	// this happens (e.g.) for 'minus_pr' if it encounters a prefix-minus
	result = resolve(env, source);
      }
      else {
	// pass the unknown exception on
	throw(excp);
      }
    }
    return result;
  }
  else
    // no prule to apply, thus...
    return resolve_dots_and_fn_calls(env, source);
}


any_t resolve_each(any_t env, list_t source)
{
  list_t sink = list_new();
  any_t head = 0;
  while ((head = list_popfirst(source))) {
    if (ptype(head) == RHA_list_t)
      list_append(sink, resolve(env, UNWRAP_PTR(RHA_list_t, head)));
    else
      list_append(sink, head);
  }
  return WRAP_PTR(RHA_tuple_t, list_to_tuple(sink));
}


// call a specific prule
//
// applies the associated function, resolves the arguments, and
// resolve possible macro calls.
any_t call_prule(any_t env, list_t source, any_t prule)
{
  // resolve prule by constructing a call to that prule
  int tlen = 3;
  tuple_t prule_call = tuple_new(tlen);
  tuple_set(prule_call, 0, prule);
  tuple_set(prule_call, 1, env);
  tuple_set(prule_call, 2, WRAP_PTR(RHA_list_t, source));

  // the list containing the parse tree
  // run the prule itself to resolve it
  any_t expr = call_fun(env, prule_call);

  // check what we got
  if (ptype(expr) == RHA_tuple_t) {
    tuple_t t = UNWRAP_PTR(RHA_tuple_t, expr);
    // check the resulting tuple
    if ((tuple_len(t)==0) || (ptype(tuple_get(t, 0))!=RHA_symbol_t))
      rha_error("(parsing) prule must create function call with function symbol");

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


// find the prule with the lowest precendence
static any_t find_best_prule(any_t env, list_t source)
{
  // loop over all entries and check the symbols whether they are prules 

  double best_priority = 0.0;  // initially the priority is like functions
  any_t best_prule  = 0;    // the object containing the best prule
  list_it_t it;
  for (it = list_begin(source); !list_done(it); glist_next(it)) {
    any_t symbol_obj = list_get(it);

    if (ptype(symbol_obj) == RHA_symbol_t) {
      any_t prule = lookup(prules, UNWRAP_SYMBOL(symbol_obj));
      double priority = get_priority(prule);

      if(!isnan(priority) && priority > best_priority) {
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

// return double value contained in prule.priority_sym or NAN if it
// does not exist
static double get_priority(any_t prule)
{
  if (prule) {
    any_t prior = lookup(prule, priority_sym);
    if (prior && (ptype(prior) == RHA_real_t))
      return UNWRAP_REAL(prior);
  }
  return NAN;
}

//----------------------------------------------------------------------



any_t resolve_dots_and_fn_calls(any_t env, list_t source)
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

  // now also for literals:
  //   list[1,2,3]
  //   [1,2,3]
  //   mat[1,2,3]
  
  // ok there is more:
  //   list().append(17);
  // more precisely:
  //   (1) the first thing can be anything but a 'dot'
  //   (2) a series of dots and argument lists

  if (list_len(source) == 0)
    rha_error("(parsing) missing expression");

  // get the first piece and resolve it
  any_t obj = list_popfirst(source);
  assert(obj);
  if ((ptype(obj)==RHA_symbol_t) && UNWRAP_SYMBOL(obj)==dot_sym)
    rha_error("(parsing) something else than dot expected");
  else if (ptype(obj) == RHA_list_t) {
    list_t l = 0;
    if (is_marked_list(rounded_sym, obj)) {
      // must be a grouped expression, so remove the brackets
      l = split_rounded_list_obj(obj);
      if (list_len(l) != 1)
	rha_error("grouped expression expected, found", obj);
    }
    //    else if (is_marked_list(squared_sym, obj)) {
    //  
    //}
    else
      l = UNWRAP_PTR(RHA_list_t, obj);
    // and resolve it
    obj = resolve(env, l);
  }

  // start the expression
  any_t expr = obj;

  // we need to keep a flag which tells us later whether we have the
  // usual function call or whether the last dot results in a
  // slotcall. 
  bool dotted = false;
  while ((obj = list_popfirst(source))) {
    if ((ptype(obj) == RHA_symbol_t) && UNWRAP_SYMBOL(obj)==dot_sym) {
      // (2) dotted expression

      // get the next object as well which must be a symbol
      obj = list_popfirst(source);
      if (!obj || (ptype(obj) != RHA_symbol_t))
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
      expr = WRAP_PTR(RHA_tuple_t, t);
      continue;
    }
    else if (is_marked_list(rounded_sym, obj)) {
      // (3) function/method call
      list_t sink = list_new();
      list_t signature = split_rounded_list_obj(obj);
      any_t arg = 0;
      while ((arg = list_popfirst(signature))) {
	assert(arg && ptype(arg)==RHA_list_t);
	list_append(sink, resolve(env, UNWRAP_PTR(RHA_list_t, arg)));
      }
      if (dotted) {
	// construct a method call
	dotted = false; // wait for the next dot
	// for a.f(x,y) we have by now two ingredients:
	// (i)  expr == (eval a \f)
	// (ii) sink == [x y]
	// from those we construct
	//      (callslot a \f x y)
	tuple_t t = UNWRAP_PTR(RHA_tuple_t, expr);
	list_prepend(sink, tuple_get(t, 2));           // prepend '\f'
	list_prepend(sink, tuple_get(t, 1));           // prepend 'a'
	list_prepend(sink, WRAP_SYMBOL(callslot_sym)); // prepend 'callslot'
	expr = WRAP_PTR(RHA_tuple_t, list_to_tuple(sink));
	continue;
      }
      else {
	// construct a plain function call
	list_prepend(sink, expr);
	expr = resolve_macro(env, list_to_tuple(sink));
	continue;
      }
    }
    else if (is_marked_list(squared_sym, obj)) {
      // (3) named literal
      tuple_t literal = squared_pr(env, UNWRAP_PTR(RHA_list_t, obj));
      // (callslot lhs \literal env [1 ,, 2 ,, 3])
      tuple_t t = tuple_new(5);
      tuple_set(t, 0, WRAP_SYMBOL(callslot_sym));
      tuple_set(t, 1, expr);
      tuple_set(t, 2, quoted(tuple_get(literal, 0)));
      tuple_set(t, 3, tuple_get(literal, 1));
      tuple_set(t, 4, tuple_get(literal, 2));
      expr = WRAP_PTR(RHA_tuple_t, t);
      continue;
    }
    else {
      rha_error("(parsing) argument list or dot expected, "
		"found \"%o\"", obj);
    }
  }

  //debug("return (resolve_dots_and_fn_calls) %o\n", expr);
  return expr;
}


any_t resolve_pattern(any_t env, list_t source)
{
  assert(false); // don't use this!

  // right now we only look for an optional type and a symbol
  list_t lhs = list_chop_first(source, symbol_new(":"));
  if (list_len(lhs) == 0)
    rha_error("(parsing) pattern can't start with colon");
  any_t theliteral = 0;
  any_t thetype = 0;
  if (list_len(source) > 0) {
    // there is a type (we evaluate it later in 'eval')
    thetype = resolve(env, lhs);
    theliteral = resolve(env, source);
  }
  else
    theliteral = resolve(env, lhs);

  if (thetype)
    return WRAP_PTR(RHA_tuple_t, tuple_make(2, quoted(theliteral), thetype));
  else
    return WRAP_PTR(RHA_tuple_t, tuple_make(1, quoted(theliteral)));
}

any_t resolve_patterns(any_t env, list_t source)
{
  assert(false); // don't use this!

  tuple_t t = tuple_new(list_len(source));
  any_t entry = 0;
  int i = 0;
  while ((entry = list_popfirst(source))) {
    // resolve the entries as patterns
    assert(entry && ptype(entry)==RHA_list_t);
    tuple_set(t, i++, resolve_pattern(env, UNWRAP_PTR(RHA_list_t, entry)));
  }
  return WRAP_PTR(RHA_tuple_t, tuple_make(4, WRAP_SYMBOL(symbol_new("map_fn")),
				      WRAP_SYMBOL(local_sym),
				      WRAP_SYMBOL(symbol_new("pattern")),
				      quoted(WRAP_PTR(RHA_tuple_t, t))));
}



//----------------------------------------------------------------------
// resolve macros

// resolve macros
//
// check whether it is a function call, and called function is a macro
// (having slot "ismacro".) Then, return function body with arguments
// replaced.
any_t resolve_macro(any_t env, tuple_t t)
{
  // It might just be an empty tuple
  int tlen = tuple_len(t);
  if (tlen == 0) 
    return WRAP_PTR(RHA_tuple_t, t);

  // or a tuple which does not contain a symbol at position 0
  any_t t0 = tuple_get(t, 0);
  if (ptype(t0) != RHA_symbol_t)
    return WRAP_PTR(RHA_tuple_t, t);
  symbol_t s = UNWRAP_SYMBOL(t0);

  // okay, we're ready to gather all information and perform the
  // replacement
  any_t macro, macro_body, _macro_args;  

  if ( (macro = lookup(env, s))
       && lookup(macro, ismacro_sym) ) {
    if ( (macro_body = lookup(macro, fnbody_sym))
	 && (_macro_args = lookup(macro, argnames_sym)) ) {
      if (ptype(_macro_args)!=RHA_tuple_t)
	rha_error("(parsing) broken macroargs");
      tuple_t macro_args = UNWRAP_PTR(RHA_tuple_t, _macro_args);
      
      if (tuple_len(macro_args) != tlen-1)
	rha_error("(parsing) wrong macro argument number (expected %d, got %d)", 
		  tuple_len(macro_args), tlen-1);
      // replace all symbols in macro_body
      for (int i=1; i<tlen; i++) {
	macro_body = replace_expr(macro_body, 
				  UNWRAP_SYMBOL(tuple_get(macro_args, i-1)),
				  tuple_get(t, i));
      }
      return macro_body;
    }
    else { // no fnbody or argnames found
      rha_error("(parsing) ismacro found but no function found");
      return 0;
    }
  }
  else // not found or ismacro missing
    return WRAP_PTR(RHA_tuple_t, t);
}


// replace all occurences of 's' in 'expr' by 'sub'
static any_t replace_expr(any_t expr, symbol_t s, any_t sub)
{
  if (ptype(expr) == RHA_symbol_t) {
    symbol_t sy = UNWRAP_SYMBOL(expr);
    if (sy == s)
      return sub;
  }
  else if (ptype(expr) == RHA_tuple_t) {
    tuple_t t = UNWRAP_PTR(RHA_tuple_t, expr);
    // create a new tuple
    int tlen = tuple_len(t);
    tuple_t newt = tuple_new(tlen);
    for (int i=0; i<tuple_len(t); i++)
      tuple_set(newt, i, replace_expr(tuple_get(t, i), s, sub));
    return WRAP_PTR(RHA_tuple_t, newt);
  }
  return expr;
}


