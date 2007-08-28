#include "prules.h"

#include "core.h"
#include "list_fn.h"
#include "tuple_fn.h"
#include "messages.h"
#include "utils.h"
#include "assert.h"
#include "debug.h"

// additional symbols used here
symbol_t prefix_minus_sym = 0;

// prules take a white-space list and return a function call!
// prules must not return a macro call!

// the prule symbols as they appear in Rhabarber
symbol_t plusplus_sym = 0;
symbol_t minusminus_sym = 0;
symbol_t not_sym = 0;
symbol_t colon_sym = 0;    // for slices and typing 
symbol_t in_sym = 0;       // for element check
symbol_t divide_sym = 0;
symbol_t times_sym = 0;
symbol_t minus_sym = 0;
symbol_t plus_sym = 0;
symbol_t less_sym = 0;
symbol_t lessequal_sym = 0;
symbol_t greater_sym = 0;
symbol_t greaterequal_sym = 0;
symbol_t equalequal_sym = 0;
symbol_t notequal_sym = 0;
symbol_t and_sym = 0;
symbol_t or_sym = 0;
symbol_t return_sym = 0;
symbol_t deliver_sym = 0;
symbol_t break_sym = 0;
symbol_t throw_sym = 0;
symbol_t if_sym = 0;
symbol_t else_sym = 0;
symbol_t try_sym = 0;
symbol_t catch_sym = 0;
symbol_t while_sym = 0;
symbol_t for_sym = 0;
symbol_t fn_sym = 0;
symbol_t macro_sym = 0;
symbol_t equal_sym = 0;
symbol_t plusequal_sym = 0;
symbol_t minusequal_sym = 0;
symbol_t timesequal_sym = 0;
symbol_t divideequal_sym = 0;
symbol_t quote_sym = 0;
symbol_t semicolon_sym = 0;
symbol_t comma_sym = 0;


glist_t assign_sym_list; // a list with = += -= *= /=
glist_t cmp_sym_list;    // a list with < <= > >= == !=
glist_t sec_sym_list;    // a list with 'else', 'catch'

#define MAKE_PRULES(ttt, prio) f = lookup(module, ttt ## _pr_sym);	\
  assign(f, priority_sym, WRAP_REAL(prio));				\
  assign(module, ttt ## _sym, f);

void prules_init(object_t root, object_t module)
{
  // additional symbols
  prefix_minus_sym = symbol_new("prefix_minus");

  // initialize prule symbols
  // the string must show the form that appears in Rhabarber
  // and can be different from the function name in C
  plusplus_sym     = symbol_new("++");
  minusminus_sym   = symbol_new("--");
  not_sym          = symbol_new("!");
  colon_sym        = symbol_new(":");
  in_sym           = symbol_new("in");
  divide_sym       = symbol_new("/");
  times_sym        = symbol_new("*");
  minus_sym        = symbol_new("-");
  plus_sym         = symbol_new("+");
  less_sym         = symbol_new("<");
  lessequal_sym    = symbol_new("<=");
  greater_sym      = symbol_new(">");
  greaterequal_sym = symbol_new(">=");
  equalequal_sym   = symbol_new("==");
  notequal_sym     = symbol_new("!=");
  and_sym          = symbol_new("&&");
  or_sym           = symbol_new("||");
  return_sym       = symbol_new("return");
  deliver_sym      = symbol_new("deliver");
  break_sym        = symbol_new("break");
  throw_sym        = symbol_new("throw");
  if_sym           = symbol_new("if");
  else_sym         = symbol_new("else");
  try_sym          = symbol_new("try");
  catch_sym        = symbol_new("catch");
  while_sym        = symbol_new("while");
  for_sym          = symbol_new("for");
  fn_sym           = symbol_new("fn");
  macro_sym        = symbol_new("macro");
  equal_sym        = symbol_new("=");
  plusequal_sym    = symbol_new("+=");
  minusequal_sym   = symbol_new("-=");
  timesequal_sym   = symbol_new("*=");
  divideequal_sym  = symbol_new("/=");
  quote_sym        = symbol_new("\\");  // note the quoted slash!
  semicolon_sym    = symbol_new(";");
  comma_sym        = symbol_new(",");

  // make a list of the assign prule symbols
  glist_init(&assign_sym_list);
  glist_append(&assign_sym_list, equal_sym);
  glist_append(&assign_sym_list, plusequal_sym);
  glist_append(&assign_sym_list, minusequal_sym);
  glist_append(&assign_sym_list, timesequal_sym);
  glist_append(&assign_sym_list, divideequal_sym);

  // make a list of the comparison prule symbol
  glist_init(&cmp_sym_list);
  glist_append(&cmp_sym_list, less_sym);
  glist_append(&cmp_sym_list, lessequal_sym);
  glist_append(&cmp_sym_list, greater_sym);
  glist_append(&cmp_sym_list, greaterequal_sym);
  glist_append(&cmp_sym_list, equalequal_sym);
  glist_append(&cmp_sym_list, notequal_sym);

  // make a list of the second order keywords
  glist_init(&sec_sym_list);
  glist_append(&sec_sym_list, else_sym);
  glist_append(&sec_sym_list, catch_sym);

  // add priority slots for all prules
  // and add those prules also as symbols
  // the strongest binding is the one with the smallest number!
  // note that 'dot' and funcalls are the strongest
  object_t f = 0;
  MAKE_PRULES(prefix_minus, 1.0);
  MAKE_PRULES(plusplus, 1.0);   // increment, decrement
  MAKE_PRULES(minusminus, 1.0);
  MAKE_PRULES(colon, 2.0);     // for slicing and typing
  MAKE_PRULES(divide, 3.0);     // arithemics
  MAKE_PRULES(times, 4.0);
  MAKE_PRULES(minus, 5.0);
  MAKE_PRULES(plus, 6.0);
  MAKE_PRULES(in, 6.5);         // element test for iterables
  MAKE_PRULES(less, 7.0);       // comparison
  MAKE_PRULES(lessequal, 7.0);
  MAKE_PRULES(greater, 7.0);
  MAKE_PRULES(greaterequal, 7.0);
  MAKE_PRULES(equalequal, 7.0);
  MAKE_PRULES(notequal, 7.0);
  MAKE_PRULES(not, 2.5);        // logic
  MAKE_PRULES(and, 8.0);        // logic
  MAKE_PRULES(or, 9.0);         // logic
  MAKE_PRULES(return, 10.0);
  MAKE_PRULES(deliver, 10.0);
  MAKE_PRULES(break, 10.0);
  MAKE_PRULES(throw, 10.0);
  MAKE_PRULES(if, 10.0);
  MAKE_PRULES(try, 10.0);
  MAKE_PRULES(while, 10.0);
  MAKE_PRULES(for, 10.0);
  MAKE_PRULES(fn, 10.0);
  MAKE_PRULES(macro, 10.0);
  // note that 'assignments' must have the same priority as freefix
  // forms, consider:
  //
  //    x = if (true) 17 else 42;       x = (if (true) 17 else 42)
  //    if (true) x = 17 else x = 42;   if (true) (x=17) else (x=42)
  MAKE_PRULES(equal, 10.0);     // assignments
  MAKE_PRULES(plusequal, 10.0);
  MAKE_PRULES(minusequal, 10.0);
  MAKE_PRULES(timesequal, 10.0);
  MAKE_PRULES(divideequal, 10.0);
  // note that 'quote' must have the same priority as freefix
  // forms, consider:
  //
  //    x = \peter;         x = (\peter);
  //    \x = peter;         \(x = peter);
  MAKE_PRULES(quote, 10.0);     // quote
  MAKE_PRULES(curlied, 20.0);   // curlied brackets
  MAKE_PRULES(squared, 20.0);   // squared brackets
}


// function to define simple prules
// NOT YET
object_t prule_new_infix(){ return 0; }
object_t prule_new_prefix(){ return 0; }
object_t prule_new_postfix(){ return 0; }
object_t prule_new_freefix(){ return 0; }


// forward declaration of helper functions

tuple_t resolve_incdec_prule(object_t env, list_t parsetree);
tuple_t resolve_infix_prule(object_t env, list_t parsetree,
			    symbol_t prule_sym, symbol_t fun_sym, 
			    bool_t left_binding);
tuple_t resolve_infix_prule_list(object_t env, list_t parsetree, 
				 glist_t *prule_sym_list, 
				 symbol_t fun_sym, bool_t left_binding);
tuple_t resolve_assign_prule(object_t env, list_t parsetree, symbol_t
			     prule_sym, glist_t *assign_sym_list);
tuple_t resolve_cmp_prule(object_t env, list_t parsetree);
tuple_t resolve_freefix_prule1(object_t env, list_t parsetree,
			       symbol_t fun_sym,
			       symbol_t key1, int_t narg1);
tuple_t resolve_freefix_prule2(object_t env, list_t parsetree,
			       symbol_t fun_sym,
			       symbol_t key1, int_t narg1, 
			       symbol_t key2, int_t narg2);


// more auxiliaries
bool is_symbol(symbol_t a_symbol, object_t expr);
object_t quoted(object_t obj);
bool_t   is_marked_list(symbol_t mark, object_t obj);

static bool_t   is_second_order_keyword(object_t obj);
static object_t copy_expr(object_t expr);


/**********************************************************************
 * 
 * All prules defined in C
 *
 **********************************************************************/

// here they should end on '_pr'
//
// the reason is to allow some prule-names in C to be different from
// their name in rhabarber.  this is important for, e.g.:
//
//    RHA-NAME C-NAME     C-IMPLEMENTATION
//    *        times_pr   times_fn
//    /        divide_pr  divide_fn
//    return   return_pr  return_fn
//
// the keyword case is the reason why '_pr' is necessary
// 
// the signature is 
//
//    tuple_t plusplus_pr(object_t env, list_t parsetree)
//
// the 'env' is needed only if the prule resolves its arg itsself
// (e.g. in minus_pr).
tuple_t plusplus_pr(object_t env, list_t parsetree) 
{ 
  return resolve_incdec_prule(env, parsetree); 
}

tuple_t minusminus_pr(object_t env, list_t parsetree) 
{ 
  return resolve_incdec_prule(env, parsetree); 
}

tuple_t not_pr(object_t env, list_t parsetree) 
{ 
  list_popfirst(parsetree);	
  tuple_t t = tuple_new(2);					
  tuple_set(t, 0, WRAP_SYMBOL(not_fn_sym));			
  if (list_len(parsetree) > 0)					
    tuple_set(t, 1, resolve(env, parsetree));
  else								
    rha_error("(parsing) 'not' requires a single argument");
  return t;
}

tuple_t divide_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(env, parsetree, divide_sym, divide_fn_sym, LEFT_BIND);
}

tuple_t times_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(env, parsetree, times_sym, times_fn_sym, LEFT_BIND);
}


tuple_t prefix_minus_pr(object_t env, list_t parsetree) 
{
  if (list_len(parsetree) < 2)
    rha_error("(parsing) prefix-minus requires argument.");
  list_popfirst(parsetree);
  return tuple_make(2, WRAP_SYMBOL(neg_fn_sym), resolve(env, parsetree));
}

tuple_t minus_pr(object_t env, list_t parsetree) 
{
  // here we also have to take care of the prefix minus, 
  // which actually has a higher priority (== lower precedence)
  
  // suppose we are trying to resolve a minus-sign since it had higher
  // priority than a times sign, however, actually, the minus-sign is
  // a prefix minus and thus should have lower priority
  
  // (1) how can we identify that case?
  // look whether it is the left-most tuple-entry
  // if so, we replace it by a special symbol which will trigger a
  // prefix prule later on
  object_t first = list_first(parsetree);
  if ((ptype(first)==SYMBOL_T) && (UNWRAP_SYMBOL(first)==minus_sym)) {
    // replace it
    list_popfirst(parsetree);
    list_prepend(parsetree, WRAP_SYMBOL(prefix_minus_sym));
    // start over elsewhere!
    throw(prule_failed_excp);
  }
  return resolve_infix_prule(env, parsetree, minus_sym, minus_fn_sym, LEFT_BIND);
}

tuple_t plus_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(env, parsetree, plus_sym, plus_fn_sym, LEFT_BIND);
}

tuple_t in_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(env, parsetree, in_sym, symbol_new("in_fn"), LEFT_BIND);
}

tuple_t colon_pr(object_t env, list_t parsetree) 
{
  // how about rightbinding?
  // a:(b:c) would be checking type of the result of b:c
  // however, doesn't make much sense!
  return resolve_infix_prule(env, parsetree, colon_sym, symbol_new("colon_fn"), RIGHT_BIND);
}

tuple_t less_pr(object_t env, list_t parsetree) 
{ 
  return resolve_cmp_prule(env, parsetree); 
}

tuple_t lessequal_pr(object_t env, list_t parsetree) 
{ 
  return resolve_cmp_prule(env, parsetree); 
}

tuple_t greater_pr(object_t env, list_t parsetree) 
{
  return resolve_cmp_prule(env, parsetree); 
}

tuple_t greaterequal_pr(object_t env, list_t parsetree) 
{ 
  return resolve_cmp_prule(env, parsetree); 
}

tuple_t equalequal_pr(object_t env, list_t parsetree) 
{ 
  return resolve_cmp_prule(env, parsetree); 
}

tuple_t notequal_pr(object_t env, list_t parsetree) 
{ 
  return resolve_cmp_prule(env, parsetree); 
}

tuple_t and_pr(object_t env, list_t parsetree) 
{ 
  return resolve_infix_prule(env, parsetree, and_sym, and_fn_sym, LEFT_BIND);
}

tuple_t or_pr(object_t env, list_t parsetree) 
{ 
  return resolve_infix_prule(env, parsetree, or_sym, or_fn_sym, LEFT_BIND);
}

#define HANDLE_STUFF_LIKE_RETURN(ttt) list_popfirst(parsetree);	\
  tuple_t t = tuple_new(2);					\
  tuple_set(t, 0, WRAP_SYMBOL(ttt ## _fn_sym));			\
  if (list_len(parsetree) > 0)					\
    tuple_set(t, 1, resolve(env, parsetree));		\
  else								\
    tuple_set(t, 1, 0);						\
  return t;

tuple_t return_pr(object_t env, list_t parsetree) 
{ 
  HANDLE_STUFF_LIKE_RETURN(return)
}

tuple_t deliver_pr(object_t env, list_t parsetree)
{ 
  HANDLE_STUFF_LIKE_RETURN(deliver)
}

tuple_t break_pr(object_t env, list_t parsetree)
{ 
  HANDLE_STUFF_LIKE_RETURN(break)
}

tuple_t throw_pr(object_t env, list_t parsetree)
{
  HANDLE_STUFF_LIKE_RETURN(throw)
}

tuple_t if_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule2(env, parsetree, if_fn_sym,
					 if_sym, 2, else_sym, 1);
  assert(tuple_len(pre_t) == 4);
  tuple_t t = tuple_new(5);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  object_t obj = tuple_get(pre_t, 1);
  assert(obj && ptype(obj)==LIST_T);  // otherwise bug in resolve_freefix*
  list_t obj_l = UNWRAP_PTR(LIST_T, obj);
  if (list_len(obj_l) != 1)
    rha_error("(parsing) condition expected, found %o", obj);
  obj = list_popfirst(obj_l);
  assert(obj && ptype(obj) == LIST_T);
  object_t cond = resolve(env, UNWRAP_PTR(LIST_T, obj));
  tuple_set(t, 2, cond);
  tuple_set(t, 3, quoted(tuple_get(pre_t, 2)));
  tuple_set(t, 4, quoted(tuple_get(pre_t, 3)));
  return t;
}

tuple_t try_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule2(env, parsetree, try_fn_sym, try_sym, 1, catch_sym, 2);
  assert(tuple_len(pre_t) == 4);
  tuple_t t = tuple_new(5);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  tuple_set(t, 2, quoted(tuple_get(pre_t, 1)));
  object_t obj = tuple_get(pre_t, 2);
  assert(obj && ptype(obj)==LIST_T);  // otherwise bug in resolve_freefix*
  list_t obj_l = UNWRAP_PTR(LIST_T, obj);
  if (list_len(obj_l) != 1)
    rha_error("(parsing) symbol for exception expected, found %o", obj);
  object_t s = list_popfirst(obj_l);
  if (ptype(s) != SYMBOL_T)
    rha_error("in 'try 17 catch (x) 42', 'x' must be a symbol");
  tuple_set(t, 3, quoted(s));
  tuple_set(t, 4, quoted(tuple_get(pre_t, 3)));
  return t;
}

tuple_t while_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, while_fn_sym,
					 while_sym, 2);
  assert(tuple_len(pre_t) == 3);
  tuple_t t = tuple_new(4);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  object_t obj = tuple_get(pre_t, 1);
  assert(obj && ptype(obj)==LIST_T);  // otherwise bug in resolve_freefix*
  list_t obj_l = UNWRAP_PTR(LIST_T, obj);
  if (list_len(obj_l) != 1)
    rha_error("(parsing) condition expected, found %o", obj);
  obj = list_popfirst(obj_l);
  assert(obj && ptype(obj) == LIST_T);
  object_t cond = resolve(env, UNWRAP_PTR(LIST_T, obj));
  tuple_set(t, 2, quoted(cond));
  tuple_set(t, 3, quoted(tuple_get(pre_t, 2)));
  return t;
}

tuple_t for_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, for_fn_sym, for_sym, 2);
  // the second argument (e.g. (x in l) must be splitted into two
  assert(tuple_len(pre_t) == 3);
  tuple_t t = tuple_new(5);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  // check whether the first arg has the right form, i.e. "x in expr"
  object_t obj = tuple_get(pre_t, 1);
  assert(obj && ptype(obj)==LIST_T);  // otherwise bug in resolve_freefix*
  list_t obj_l = UNWRAP_PTR(LIST_T, obj);
  if (list_len(obj_l) != 1)
    rha_error("(parsing) second arg to 'for' must look like (x in l), found %o", obj);
  object_t obj_t0 = list_popfirst(obj_l);
  assert(obj_t0 && ptype(obj_t0)==LIST_T);  // otherwise bug in resolve_freefix*
  list_t part = UNWRAP_PTR(LIST_T, obj_t0);
  if (list_len(part) < 3)
    rha_error("(parsing) second arg to 'for' must look like (x in l), found %o", obj);
  object_t s = list_popfirst(part);
  if (ptype(s) != SYMBOL_T)
    rha_error("(parsing) symbol expected, found %o", s);
  object_t in_o = list_popfirst(part);
  if (!in_o || !is_symbol(in_sym, in_o))
    rha_error("(parsing) 'in' expected, found %o", in_o);
  object_t l = resolve(env, part);
  tuple_set(t, 2, quoted(s));
  tuple_set(t, 3, l);
  tuple_set(t, 4, quoted(tuple_get(pre_t, 2)));
  return t;
}


tuple_t fn_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, fn_fn_sym, fn_sym, 2);
  assert(tuple_len(pre_t) == 3);
  tuple_t t = tuple_new(4);
  tuple_set(t, 0, WRAP_SYMBOL(fn_fn_sym));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  object_t obj = tuple_get(pre_t, 1);
  assert(obj && ptype(obj)==LIST_T);  // otherwise bug in resolve_freefix*
  list_t obj_l = UNWRAP_PTR(LIST_T, obj);
  tuple_set(t, 2, quoted(resolve_patterns(env, obj_l)));
  tuple_set(t, 3, quoted(tuple_get(pre_t, 2)));
  //debug("(fn_pr) %o\n", WRAP_PTR(TUPLE_T, t));
  return t;
}

tuple_t macro_pr(object_t env, list_t parsetree)
{ 
  // currently this functions contains mainly a dirty hack!
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, macro_fn_sym, macro_sym, 2);
  assert(tuple_len(pre_t) == 3);
  tuple_t t = tuple_new(3);
  tuple_set(t, 0, WRAP_SYMBOL(macro_fn_sym));
  object_t obj = tuple_get(pre_t, 1);
  assert(obj && ptype(obj)==LIST_T);  // otherwise bug in resolve_freefix*
  tuple_t obj_t = list_to_tuple(UNWRAP_PTR(LIST_T, obj));
  for (int i = 0; i < tuple_len(obj_t); i++) {
    // simply check whether the args are symbols
    object_t entry = tuple_get(obj_t, i);
    assert(entry && ptype(entry)==LIST_T);
    object_t arg = resolve(env, UNWRAP_PTR(LIST_T, entry));
    if (!arg || ptype(arg)!=SYMBOL_T)
      rha_error("(parsing) macro expects symbol, found", arg);
    tuple_set(obj_t, i, arg);
  }
  tuple_set(t, 1, quoted(WRAP_PTR(TUPLE_T, obj_t)));
  tuple_set(t, 2, quoted(tuple_get(pre_t, 2)));
  //debug("(macro_pr) %o\n", WRAP_PTR(TUPLE_T, t));
  return t;
}

tuple_t equal_pr(object_t env, list_t parsetree) {
  return resolve_assign_prule(env, parsetree, equal_sym, &assign_sym_list);
}

tuple_t plusequal_pr(object_t env, list_t parsetree) {
  return resolve_assign_prule(env, parsetree, plusequal_sym, &assign_sym_list);
}

tuple_t minusequal_pr(object_t env, list_t parsetree) {
  return resolve_assign_prule(env, parsetree, minusequal_sym, &assign_sym_list);
}

tuple_t timesequal_pr(object_t env, list_t parsetree) {
  return resolve_assign_prule(env, parsetree, timesequal_sym, &assign_sym_list);
}

tuple_t divideequal_pr(object_t env, list_t parsetree) {
  return resolve_assign_prule(env, parsetree, divideequal_sym, &assign_sym_list);
}

tuple_t quote_pr(object_t env, list_t parsetree)
{
  list_popfirst(parsetree);	
  tuple_t t = tuple_new(2);					
  tuple_set(t, 0, WRAP_SYMBOL(quote_sym));			
  if (list_len(parsetree) > 0) {
    if (is_symbol(quote_sym, list_first(parsetree))) {
      // this allows to avoid resolving prules at all
      list_popfirst(parsetree);
      tuple_set(t, 1, WRAP_PTR(LIST_T, parsetree));
    }
    else
      // resolve prules
      tuple_set(t, 1, resolve(env, parsetree));
  }
  else								
    tuple_set(t, 1, 0);
  return t;
}




//----------------------------------------------------------------------
// resolving a curlied list (code blocks)

// take care of curlied lists, returns a tuple
//
// code blocks  -->  returns a tuple_t
// for example: { x = 1; y = 7; deliver 5; { a=5; } x=5 }
// split by semicolon, comma is not allowed
tuple_t curlied_pr(object_t env, list_t parsetree)
{
  //debug("curlied_pr(%o, %o)\n", env, WRAP_PTR(LIST_T, parsetree));
  assert(list_len(parsetree) > 0);
  object_t head = list_popfirst(parsetree);  // pop off curlied_sym
  assert(head && is_symbol(curlied_sym, head));

  list_t sink = list_new();
  list_t part = list_new();
  object_t obj = list_popfirst(parsetree);
  while (obj) {
    if (is_symbol(semicolon_sym, obj)) {
      // split only if the next one is not a second order keyword like
      // 'else' or 'catch'
      obj = list_popfirst(parsetree);
      if (!obj) break;
      if (!is_second_order_keyword(obj) && list_len(part)>0) {
	// split here
	list_append(sink, resolve(env, part));
	part = list_new();
	// ignore the semicolon
      }
      continue;
    }
    else if (is_symbol(comma_sym, obj)) {
      // this is not allowed
      rha_error("(parsing) in a 'curlied' expression no comma allowed");
      assert(1==0);
    }
    else if (is_marked_list(curlied_sym, obj)){
      // a code block
      // in case there is not a keyword like 'else' or 'catch'
      // following, we split here as well
      list_append(part, obj);
      // look at the next one
      obj = list_popfirst(parsetree);
      if (!obj) break;
      if (!is_second_order_keyword(obj)) {
	// split
	list_append(sink, resolve(env, part));
	part = list_new();
      }
      continue;
    }
    else {
      list_append(part, obj);
    }
    obj = list_popfirst(parsetree);
  }
  if (list_len(part)>0)
    list_append(sink, resolve(env, part));
  list_prepend(sink, WRAP_SYMBOL(do_sym));
  return list_to_tuple(sink);
}

tuple_t squared_pr(object_t env, list_t parsetree)
{
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
  object_t head = list_popfirst(parsetree);  // pop off curlied_sym
  assert(head && is_symbol(squared_sym, head));

  return tuple_make(3, WRAP_SYMBOL(literal_sym), env, 
		    quoted(WRAP_PTR(LIST_T, parsetree)));
}






/**********************************************************************
 * 
 * Helper functions for prules
 *
 **********************************************************************/

bool_t is_marked_list(symbol_t mark, object_t obj)
{
  if (ptype(obj) != LIST_T)
    return false;
  list_t l = UNWRAP_PTR(LIST_T, obj);
  if (list_len(l) == 0)
    return false;
  object_t l0 = list_first(l);
  if (!l0 || !is_symbol(mark, l0))
    return false;
  // ok, it is a marked list
  return true;
}

// check whether an expression is a specific symbol
bool is_symbol(symbol_t a_symbol, object_t expr) {
  return (ptype(expr) == SYMBOL_T)
    && (UNWRAP_SYMBOL(expr) == a_symbol);
}

static bool_t is_second_order_keyword(object_t obj)
{
  // 2nd order keywords are for example 'else' or 'catch'
  if (ptype(obj) != SYMBOL_T)
    return false;
  symbol_t s = UNWRAP_SYMBOL(obj);
  if (glist_iselementi(&sec_sym_list, s))
    return true;
  else
    return false;
}

object_t quoted(object_t obj)
{
  return WRAP_PTR(TUPLE_T, 
		  tuple_make(2, WRAP_SYMBOL(quote_sym), obj));
}

object_t copy_expr(object_t expr)
{
  // copy the structure (i.e. tuple and list structure)
  // but not the object elements
  if (ptype(expr) == TUPLE_T) {
    tuple_t t = UNWRAP_PTR(TUPLE_T, expr);
    int tlen = tuple_len(t);
    tuple_t other = tuple_new(tlen);
    for (int i=0; i<tlen; i++)
      tuple_set(other, i, copy_expr(tuple_get(t, i)));
    return WRAP_PTR(TUPLE_T, other);
  }
  else if (ptype(expr) == LIST_T) {
    list_t l = UNWRAP_PTR(LIST_T, expr);
    list_t other = list_new();
    list_it_t it;
    for (it = list_begin(l); !list_done(it); list_next(it))
      list_append(other, copy_expr(list_get(it)));
    return WRAP_PTR(LIST_T, other);
  }
  else
    return expr;
}


list_t split_rounded_list_obj(object_t list_obj)
{
  // note that 'split_rounded_list_obj' only splits and doesn't call 'resolve'!!!

  // for grouped expression, argument lists of calls and argument
  // lists of function definition
  // for example, (x+1)*4 or (x, y, z) or (int:x, real:y=0)
  // split only by comma, no semicolon is allowed

  //debug("split_rounded_list_obj(%o)\n", list_obj);
  assert(list_obj && ptype(list_obj) == LIST_T);
  list_t source = UNWRAP_PTR(LIST_T, list_obj);
  object_t head = list_popfirst(source);  // pop off curlied_sym
  assert(head && is_symbol(rounded_sym, head));

  if (list_len(source) == 0)
    return list_new();

  list_t sink = list_new();
  list_t part = list_new();
  object_t obj = 0;
  while ((obj = list_popfirst(source))) {
    if (is_symbol(comma_sym, obj)) {
      if (list_len(source)==0)
	rha_error("(parsing) trailing comma not allowed");
      // split here and ignore the comma
      if (list_len(part)==0)
	rha_error("can't start argument list with COMMA");
      list_append(sink, WRAP_PTR(LIST_T, part));
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
  if (list_len(part) == 0)
    rha_error("can't end argument list with COMMA");
  list_append(sink, WRAP_PTR(LIST_T, part));
  return sink;
}


tuple_t resolve_incdec_prule(object_t env, list_t parsetree)
{
  // notes:
  //   ++x   =>  inc(x)
  //   x++   =>  inc_copy(x)
  // what is that "copy" thing?
  //   inc_copy = fn (x) { o=copy(x); inc(x); return o };
  // similarly for 'dec'
  //debug("resolve_incdec_prule(%o)\n", WRAP_PTR(LIST_T, parsetree));

  assert(list_len(parsetree) > 0);
  if (list_len(parsetree) == 1)
    rha_error("++ and -- require an argument");
  // check whether ++ is used as prefix
  object_t first = list_first(parsetree);
  if (ptype(first) == SYMBOL_T) {
    symbol_t first_s = UNWRAP_SYMBOL(first);
    if ((first_s == plusplus_sym) || (first_s == minusminus_sym)) {
      // transform the parsetree!
      list_popfirst(parsetree);  // chop off the ++ (or --)
      tuple_t t = tuple_new(2);
      if (first_s == plusplus_sym)
	tuple_set(t, 0, WRAP_SYMBOL(inc_sym));
      else if (first_s == minusminus_sym)
	tuple_set(t, 0, WRAP_SYMBOL(dec_sym));
      else assert(1==0);
      tuple_set(t, 1, resolve(env, parsetree));
      return t;
    }
  }
  // else
  object_t last = list_last(parsetree);
  if (ptype(last) == SYMBOL_T) {
    symbol_t last_s = UNWRAP_SYMBOL(last);
    if ((last_s == plusplus_sym) || (last_s == minusminus_sym)) {
      // transform the parsetree!
      list_poplast(parsetree);  // chop off the ++ (or --)
      tuple_t t = tuple_new(2);
      if (last_s == plusplus_sym)
	tuple_set(t, 0, WRAP_SYMBOL(inc_copy_sym));
      else if (last_s == minusminus_sym)
	tuple_set(t, 0, WRAP_SYMBOL(dec_copy_sym));
      else assert(1==0);
      tuple_set(t, 1, resolve(env, parsetree));
      return t;
    }
  }
  // else
  rha_error(" ++ and -- are only prefix/praefix operators");
  assert(1==0);
  return 0; // make gcc happy
}


tuple_t resolve_infix_prule(object_t env, list_t parsetree, symbol_t prule_sym, symbol_t fun_sym, bool_t left_binding)
{
  glist_t sym_list;
  glist_init(&sym_list);
  glist_append(&sym_list, prule_sym);
  return resolve_infix_prule_list(env, parsetree, &sym_list, fun_sym, left_binding);
}


tuple_t resolve_infix_prule_list(object_t env, list_t parsetree, glist_t *prule_sym_list, symbol_t fun_sym, bool_t left_binding)
// a + (b + c)
{
  // a generic infix prule which can deal with all infix operators
  // let's manipulate the parsetree
  list_t lhs, rhs;
  if (left_binding) {
    lhs = parsetree;
    rhs = list_chop_last_list(lhs, prule_sym_list);
  }
  else {
    rhs = parsetree;
    lhs = list_chop_first_list(rhs, prule_sym_list);
  }

  if ((list_len(lhs)==0) || (list_len(rhs)==0))
    rha_error("(resolve_infix_prule) infix requires a nonempty lhs and rhs");
  tuple_t t = tuple_new(3);
  tuple_set(t, 0, WRAP_SYMBOL(fun_sym));
  tuple_set(t, 1, resolve(env, lhs));
  tuple_set(t, 2, resolve(env, rhs));
  return t;
}

tuple_t resolve_assign_prule(object_t env, list_t parsetree, symbol_t prule_sym, glist_t *assign_sym_list)
{
  // x = 0            (assign local \x 0)
  // a.x = 0          (assign a     \x 0)
  // a(z).x = 0       (assign (a z) \x 0)
  // f(x) = 0         (extend local \f \(x) local 0)
  // a.f(x) = 0       (extend a     \f \(x) local 0)
  // a(z).f(x) = 0    (extend (a z) \f \(x) local 0)

  // resolves the right-most assignment (could be = += -= *= /=)

  // debug("resolve_assign_prule(%o)\n", WRAP_PTR(LIST_T, parsetree));

  // (0) let's find the first appearance (left-most) of an assignment
  list_t lhs_l = list_chop_first(parsetree, prule_sym);
  // now we got something like 
  //         (assign lhs rhs)

  // all parts:
  object_t scope = WRAP_SYMBOL(local_sym);
  object_t symb = 0;
  object_t signature = 0;

  // (1) analyse it
  object_t lhs = 0;
  if (list_len(parsetree) == 0)
    rha_error("(parsing) no rhs for assignment");
  if (list_len(lhs_l) == 0)
    rha_error("(parsing) no lhs for assignment");
  else if (list_len(lhs_l) == 1) {
    lhs = list_popfirst(lhs_l);
  }
  else {
    // (1.1) do we have a rounded expression right-most?
    object_t right_most = list_last(lhs_l);
    if (is_marked_list(rounded_sym, right_most)) {
      // something like f(x)=17
      list_poplast(lhs_l); // remove it
      list_t signature_l = split_rounded_list_obj(right_most);
      signature = resolve_patterns(env, signature_l);
    }
    // now the right-most entry in 'lhs' should be a symbol
    // if not we get an error later on
    lhs = resolve(env, lhs_l);
  }

  // (2) there are two cases for the lhs:
  // (2.1) x       ->  lhs == x
  // (2.2) a.x     ->  lhs == (eval a \x)
  
  if (ptype(lhs) == SYMBOL_T) {
    symb = quoted(lhs);
  }
  else if (ptype(lhs) == TUPLE_T) {
    tuple_t t = UNWRAP_PTR(TUPLE_T, lhs);
    int_t tlen = tuple_len(t);
    if (tlen != 3)
	rha_error("(parsing) can't assign to ", lhs);
    object_t t0 = tuple_get(t, 0);
    if (!t0 || ptype(t0) != SYMBOL_T || UNWRAP_SYMBOL(t0)!=eval_sym)
      rha_error("(parsing) can't assign to ", lhs);
    scope = tuple_get(t, 1);
    symb  = tuple_get(t, 2);
  }
  else
    rha_error("(parsing) can't assign to ", lhs);

  // (2) to build the RHS we check whether the 'prule_sym' was
  // something else than 'equal_sym'
  object_t rhs = resolve(env, parsetree);

  // finally generate the call
  tuple_t t = 0;
  if (signature) {
    // extend
    if (prule_sym != equal_sym)
      rha_error("(parsing) += and friends only allowed for simple assignments");
    t = tuple_new(6);
    tuple_set(t, 0, WRAP_SYMBOL(extend_sym));
    tuple_set(t, 1, scope);      // later on calling scope
    tuple_set(t, 2, symb);
    tuple_set(t, 3, quoted(signature));
    tuple_set(t, 4, WRAP_SYMBOL(local_sym));
    tuple_set(t, 5, quoted(rhs));
  }
  else {
    // assign
    if (prule_sym != equal_sym) {
      assert((prule_sym==plusequal_sym) 
	     || (prule_sym==minusequal_sym)
	     || (prule_sym==timesequal_sym)
	     || (prule_sym==divideequal_sym));
      tuple_t rhs_t = tuple_new(3);
      tuple_set(rhs_t, 1, copy_expr(lhs));
      tuple_set(rhs_t, 2, rhs);
      if (prule_sym==plusequal_sym) 
	tuple_set(rhs_t, 0, WRAP_SYMBOL(plus_fn_sym));
      else if (prule_sym==minusequal_sym) 
	tuple_set(rhs_t, 0, WRAP_SYMBOL(minus_fn_sym));
      else if (prule_sym==timesequal_sym) 
	tuple_set(rhs_t, 0, WRAP_SYMBOL(times_fn_sym));
      else if (prule_sym==divideequal_sym) 
	tuple_set(rhs_t, 0, WRAP_SYMBOL(divide_fn_sym));
      else
	assert(1==0);
      rhs = WRAP_PTR(TUPLE_T, rhs_t);
    }
    t = tuple_new(4);
    tuple_set(t, 0, WRAP_SYMBOL(assign_sym));
    tuple_set(t, 1, scope);
    tuple_set(t, 2, symb);
    tuple_set(t, 3, rhs);
  }
  //debug("resolve_assign_prule returns: %o\n", WRAP_PTR(TUPLE_T, 0, t));
  return t;
}




object_t get_cmp_fn(symbol_t cmp_s)
{
  if (cmp_s==equalequal_sym)        return WRAP_SYMBOL(equalequal_fn_sym);
  else if (cmp_s==notequal_sym)     return WRAP_SYMBOL(notequal_fn_sym);
  else if (cmp_s==less_sym)         return WRAP_SYMBOL(less_fn_sym);
  else if (cmp_s==lessequal_sym)    return WRAP_SYMBOL(lessequal_fn_sym);
  else if (cmp_s==greater_sym)      return WRAP_SYMBOL(greater_fn_sym);
  else if (cmp_s==greaterequal_sym) return WRAP_SYMBOL(greaterequal_fn_sym);
  else rha_error("unkown cmp symbol \"%o\"", WRAP_SYMBOL(cmp_s));
  assert(1==0);
  return 0; // make gcc happy
}




tuple_t resolve_cmp_prule(object_t env, list_t parsetree)
// resolve stuff like a == b == c < d
{
  // (1) chop the parsetree into pieces
  list_t pieces = list_new();
  list_t parts = list_new();
  list_t cmp_l = list_new();
  object_t head;
  while ((head = list_popfirst(parsetree))) {
    if (ptype(head) == SYMBOL_T) {
      symbol_t s = UNWRAP_SYMBOL(head);
      if (glist_iselementi(&cmp_sym_list, s)) {
	if (list_len(parts) == 0)
	  rha_error("cmp symbols must not be at the beginning of an expression");
	list_append(cmp_l, get_cmp_fn(s));
	list_append(pieces, resolve(env, parts));
	parts = list_new();
	continue;
      }
    }
    list_append(parts, head);
  }
  if (list_len(parts)==0)
    rha_error("cmp symbols must not be at the end of expression");
  list_append(pieces, resolve(env, parts));

  // (2) built the expression
  object_t cmp_obj = list_popfirst(cmp_l);
  object_t part1 = list_popfirst(pieces);
  object_t part2 = list_popfirst(pieces);
  tuple_t call = tuple_make(3, cmp_obj, part1, part2);
  object_t and_fn_obj = WRAP_SYMBOL(and_fn_sym);
  while ((cmp_obj = list_popfirst(cmp_l))) {
    part1 = part2;
    part2 = list_popfirst(pieces);
    call = tuple_make(3, and_fn_obj,
		      WRAP_PTR(TUPLE_T, call),
		      WRAP_PTR(TUPLE_T, 
			       tuple_make(3, cmp_obj, part1, part2)));
  }
  return call;
}


list_t read_freefix_args(object_t env, list_t parsetree, int_t narg)
{
  if (list_len(parsetree) < narg)
    rha_error("(parsing) not enough arguments for freefix form");
  list_t args = list_new();
  for (int i=1; i<narg; i++) {
    object_t obj = list_popfirst(parsetree);
    if (!is_marked_list(rounded_sym, obj))
      rha_error("first argument of freefix form must be in round "
		"brackets, found %o", obj);
    list_append(args, WRAP_PTR(LIST_T, split_rounded_list_obj(obj)));
  }
  // last argument is the rest of the list
  list_append(args, resolve(env, parsetree));
  //debug("%o\n", WRAP_PTR(LIST_T, args));
  return args;
}

tuple_t resolve_freefix_prule1(object_t env, list_t parsetree,
			       symbol_t fun_sym,
			       symbol_t key1, int_t narg1)
{
  // note that only the last argument is resolved, all other are not!
  // the reason is that 'fn_pr' might want different stuff inside the
  // brackets than 'for_pr'

  //   for (x in l) { ... }
  // resolve_freefix_prule(env, pt, for_fn_sym, for_sym, 2);

  //   while (x>0) f(x)
  // resolve_freefix_prule(env, pt, while_fn_sym, while_sym, 2);
  //
  // note that all but the last of several arguments to keys must be
  // in rounded brackets
  //debug("resolve_freefix_prule1(%o)\n", WRAP_PTR(LIST_T, parsetree));
  object_t head = list_popfirst(parsetree);
  assert(head && is_symbol(key1, head));
  list_t call = read_freefix_args(env, parsetree, narg1);
  list_prepend(call, WRAP_SYMBOL(fun_sym));
  return list_to_tuple(call);
}


tuple_t resolve_freefix_prule2(object_t env, list_t parsetree,
			       symbol_t fun_sym,
			       symbol_t key1, int_t narg1, 
			       symbol_t key2, int_t narg2)
{
  //   if (x==0) 17
  //   if (x==0) 17 else 42
  // resolve_freefix_prule(env, pt, if_fn_sym, if_sym, 2, else_sym, 1);

  //   try f(x)
  //   try f(x) catch(excp) { }
  // resolve_freefix_prule(env, pt, try_fn_sym, try_sym, 1, catch_sym, 2);
  //
  // note that the expression with key2 is optional
  // note that all but the last of several arguments to keys must be
  // in rounded brackets
  //debug("resolve_freefix_prule2(%o)\n", WRAP_PTR(LIST_T, parsetree));
  object_t head = list_popfirst(parsetree);
  assert(head && is_symbol(key1, head));
  // split the parsetree
  int parsetree_len = list_len(parsetree);
  list_t front_l = list_chop_matching(parsetree, key1, key2);
  int front_len = list_len(front_l);
  list_t call = read_freefix_args(env, front_l, narg1);
  // check whether there was 'key2'
  if (front_len < parsetree_len) {
    // there was a key2
    list_extend(call, read_freefix_args(env, parsetree, narg2));
  }
  else
    list_append(call, 0);
  // 
  list_prepend(call, WRAP_SYMBOL(fun_sym));
  return list_to_tuple(call);
}


