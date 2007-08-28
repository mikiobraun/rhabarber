#include <assert.h>
#include <math.h>

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
#include "prules.h"
#include "excp.h"

// let's require for now that all expressions must be separated by
// semicolon, otherwise we could make a list of keywords which are
// allowed to extend expression also after a 'curlied' expression

object_t prule_failed_excp = 0; // exception

object_t prules = 0;  // the object containing all prules

symbol_t dot_sym = 0;

void parse_init(object_t root, object_t module)
{
  prule_failed_excp = excp_new("prule failed");

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

object_t resolve(object_t env, list_t source);
object_t resolve_macro(object_t env, tuple_t t);
object_t resolve_dots_and_fn_calls(object_t env, list_t source);
object_t resolve_patterns(object_t env, list_t source);


// auxillary functions
static object_t call_prule(object_t env, list_t source, object_t prule);
static object_t find_best_prule(object_t env, list_t source);
static double get_priority(object_t prule);
static object_t replace_expr(object_t expr, symbol_t s, object_t sub);


object_t parse(object_t root, string_t s)
{
  // (0) run bison code
  list_t source = rhaparsestring(s);
  if (list_len(source)==0) return 0;

  // (1) resolve prules and macros creating a big function call
  return resolve(root, source);
}

object_t parse_file(object_t root, string_t fname)
{
  // (0) run bison code
  list_t source = rhaparsefile(fname);
  
  // (1) resolve prules and macros creating a big function call
  return resolve(root, source);
}


//----------------------------------------------------------------------
// resolving prules
//
// the main workhorse for resolving prules

// find the best prule, and apply the prule
object_t resolve(object_t env, list_t source)
{
  //debug("resolve(%o, %o)\n", env, WRAP_PTR(LIST_T, source));
  object_t prule = find_best_prule(env, source);
  object_t excp = 0;
  object_t result = 0;
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

// call a specific prule
//
// applies the associated function, resolves the arguments, and
// resolve possible macro calls.
object_t call_prule(object_t env, list_t source, object_t prule)
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
static object_t find_best_prule(object_t env, list_t source)
{
  // loop over all entries and check the symbols whether they are prules 

  double best_priority = 0.0;  // initially the priority is like functions
  object_t best_prule  = 0;    // the object containing the best prule
  list_it_t it;
  for (it = list_begin(source); !list_done(it); glist_next(it)) {
    object_t symbol_obj = list_get(it);

    if (ptype(symbol_obj) == SYMBOL_T) {
      object_t prule = lookup(prules, UNWRAP_SYMBOL(symbol_obj));
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
static double get_priority(object_t prule)
{
  if (prule) {
    object_t prior = lookup(prule, priority_sym);
    if (prior && (ptype(prior) == REAL_T))
      return UNWRAP_REAL(prior);
  }
  return NAN;
}

//----------------------------------------------------------------------



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

  // get the first piece and resolve it
  object_t obj = list_popfirst(source);
  assert(obj);
  if ((ptype(obj)==SYMBOL_T) && UNWRAP_SYMBOL(obj)==dot_sym)
    rha_error("(parsing) something else than dot expected");
  else if (ptype(obj) == LIST_T) {
    list_t l = 0;
    if (is_marked_list(rounded_sym, obj)) {
      // must be a grouped expression, so remove the brackets
      l = split_rounded_list_obj(obj);
      if (list_len(l) != 1)
	rha_error("grouped expression expected, found", obj);
    }
    else
      l = UNWRAP_PTR(LIST_T, obj);
    // and resolve it
    obj = resolve(env, l);
  }

  // start the expression
  object_t expr = obj;

  // we need to keep a flag which tells us later whether we have the
  // usual function call or whether the last dot results in a
  // slotcall. 
  bool_t dotted = false;
  while ((obj = list_popfirst(source))) {
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
    else if (is_marked_list(rounded_sym, obj)) {
      // (3) function/method call
      list_t sink = list_new();
      list_t signature = split_rounded_list_obj(obj);
      object_t arg = 0;
      while ((arg = list_popfirst(signature))) {
	assert(arg && ptype(arg)==LIST_T);
	list_append(sink, resolve(env, UNWRAP_PTR(LIST_T, arg)));
      }
      if (dotted) {
	// construct a method call
	dotted = false; // wait for the next dot
	// for a.f(x,y) we have by now two ingredients:
	// (i)  expr == (eval a \f)
	// (ii) sink == [x y]
	// from those we construct
	//      (callslot a \f x y)
	tuple_t t = UNWRAP_PTR(TUPLE_T, expr);
	list_prepend(sink, tuple_get(t, 2));           // prepend '\f'
	list_prepend(sink, tuple_get(t, 1));           // prepend 'a'
	list_prepend(sink, WRAP_SYMBOL(callslot_sym)); // prepend 'callslot'
	expr = WRAP_PTR(TUPLE_T, list_to_tuple(sink));
	continue;
      }
      else {
	// construct a plain function call
	list_prepend(sink, expr);
	expr = resolve_macro(env, list_to_tuple(sink));
	continue;
      }
    }
    else {
      rha_error("(parsing) argument list or dot expected, "
		"found \"%o\"", obj);
    }
  }

  //debug("return (resolve_dots_and_fn_calls) %o\n", expr);
  return expr;
}


object_t resolve_pattern(object_t env, list_t source)
{
  // right now we only look for an optional type and a symbol
  list_t lhs = list_chop_first(source, symbol_new(":"));
  if (list_len(lhs) == 0)
    rha_error("(parsing) pattern can't start with colon");
  object_t thesymbol = 0;
  object_t thetype = 0;
  if (list_len(source) > 0) {
    // there is a type
    // in order to check it we must evaluate it
    thetype = eval(env, resolve(env, lhs));
    if (!has(thetype, symbol_new("check")))
      rha_error("the lhs of colon must implement 'check'");
    thesymbol = resolve(env, source);
  }
  else
    thesymbol = resolve(env, lhs);

  object_t pattern = new();
  assign(pattern, parent_sym, pattern_proto);
  if (thetype)
    assign(pattern, symbol_new("patterntype"), thetype);
  assign(pattern, symbol_new("patternsymbol"), thesymbol);
  return pattern;
}

object_t resolve_patterns(object_t env, list_t source)
{
  list_t sink = list_new();
  object_t entry = 0;
  while ((entry = list_popfirst(source))) {
    // resolve the entries as patterns
    assert(entry && ptype(entry)==LIST_T);
    list_append(sink, resolve_pattern(env, UNWRAP_PTR(LIST_T, entry)));
  }
  return WRAP_PTR(TUPLE_T, list_to_tuple(sink));
}



//----------------------------------------------------------------------
// resolve macros

// resolve macros
//
// check whether it is a function call, and called function is a macro
// (having slot "ismacro".) Then, return function body with arguments
// replaced.
object_t resolve_macro(object_t env, tuple_t t)
{
  // It might just be an empty tuple
  int tlen = tuple_len(t);
  if (tlen == 0) 
    return WRAP_PTR(TUPLE_T, t);

  // or a tuple which does not contain a symbol at position 0
  object_t t0 = tuple_get(t, 0);
  if (ptype(t0) != SYMBOL_T)
    return WRAP_PTR(TUPLE_T, t);
  symbol_t s = UNWRAP_SYMBOL(t0);

  // okay, we're ready to gather all information and perform the
  // replacement
  object_t macro, macro_body, _macro_args;  

  if ( (macro = lookup(env, s))
       && lookup(macro, ismacro_sym) ) {
    if ( (macro_body = lookup(macro, fnbody_sym))
	 && (_macro_args = lookup(macro, argnames_sym)) ) {
      if (ptype(_macro_args)!=TUPLE_T)
	rha_error("(parsing) broken macroargs");
      tuple_t macro_args = UNWRAP_PTR(TUPLE_T, _macro_args);
      
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
    return WRAP_PTR(TUPLE_T, t);
}


// replace all occurences of 's' in 'expr' by 'sub'
static object_t replace_expr(object_t expr, symbol_t s, object_t sub)
{
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


