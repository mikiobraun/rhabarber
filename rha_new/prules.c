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
symbol_t equal_sym = 0;
symbol_t plusequal_sym = 0;
symbol_t minusequal_sym = 0;
symbol_t timesequal_sym = 0;
symbol_t divideequal_sym = 0;
symbol_t quote_sym = 0;


static glist_t assign_sym_list; // a list with = += -= *= /=
static glist_t cmp_sym_list;    // a list with < <= > >= == !=

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
  equal_sym        = symbol_new("=");
  plusequal_sym    = symbol_new("+=");
  minusequal_sym   = symbol_new("-=");
  timesequal_sym   = symbol_new("*=");
  divideequal_sym  = symbol_new("/=");
  quote_sym        = symbol_new("\\");  // note the quoted slash!

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

  // add priority slots for all prules
  // and add those prules also as symbols
  // the strongest binding is the one with the smallest number!
  // note that 'dot' is the strongest binding and it is not a prule!
  // the second strongest is function call which are not a prule
  // either.  Both 'dots' and 'fncall' are dealt with by hand in
  // 'resolve_list_by_prule'
  object_t f = 0;
  MAKE_PRULES(prefix_minus, 1.0);
  MAKE_PRULES(plusplus, 1.0);   // increment, decrement
  MAKE_PRULES(minusminus, 1.0);
  MAKE_PRULES(not, 2.5);        // logics
  MAKE_PRULES(divide, 3.0);     // arithemics
  MAKE_PRULES(times, 4.0);
  MAKE_PRULES(minus, 5.0);
  MAKE_PRULES(plus, 6.0);
  MAKE_PRULES(colon, 6.25);     // for slicing and typing
  MAKE_PRULES(in, 6.5);         // element test for iterables
  MAKE_PRULES(less, 7.0);       // comparison
  MAKE_PRULES(lessequal, 7.0);
  MAKE_PRULES(greater, 7.0);
  MAKE_PRULES(greaterequal, 7.0);
  MAKE_PRULES(equalequal, 7.0);
  MAKE_PRULES(notequal, 7.0);
  MAKE_PRULES(and, 8.0);        // logics
  MAKE_PRULES(or, 9.0);
  MAKE_PRULES(return, 10.0);
  MAKE_PRULES(deliver, 10.0);
  MAKE_PRULES(break, 10.0);
  MAKE_PRULES(throw, 10.0);
  MAKE_PRULES(if, 10.0);
  MAKE_PRULES(try, 10.0);
  MAKE_PRULES(while, 10.0);
  MAKE_PRULES(for, 10.0);
  MAKE_PRULES(fn, 10.0);
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
}


// function to define simple prules
object_t prule_new_infix(){ return 0; }
object_t prule_new_prefix(){ return 0; }
object_t prule_new_postfix(){ return 0; }
object_t prule_new_freefix(){ return 0; }


// forward declaration of helper functions

tuple_t resolve_incdec_prule(list_t parsetree);
tuple_t resolve_infix_prule(list_t parsetree, symbol_t prule_sym, 
			    symbol_t fun_sym, bool_t left_binding);
tuple_t resolve_infix_prule_list(list_t parsetree, 
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
  return resolve_incdec_prule(parsetree); 
}

tuple_t minusminus_pr(object_t env, list_t parsetree) 
{ 
  return resolve_incdec_prule(parsetree); 
}

tuple_t not_pr(object_t env, list_t parsetree) 
{ 
  return tuple_new(0); 
}

tuple_t divide_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(parsetree, divide_sym, divide_fn_sym, LEFT_BIND);
}

tuple_t times_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(parsetree, times_sym, times_fn_sym, LEFT_BIND);
}


tuple_t prefix_minus_pr(object_t env, list_t parsetree) 
{
  if (list_len(parsetree) < 2)
    rha_error("(parsing) prefix-minus requires argument.");
  list_popfirst(parsetree);
  return tuple_make(2, WRAP_SYMBOL(neg_fn_sym), WRAP_PTR(LIST_T, parsetree));
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
  tuple_t t = resolve_infix_prule(parsetree, minus_sym, minus_fn_sym, LEFT_BIND);
  // resolve args here!
  resolve_args(env, t);
  return t;
}

tuple_t plus_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(parsetree, plus_sym, plus_fn_sym, LEFT_BIND);
}

tuple_t in_pr(object_t env, list_t parsetree) 
{
  return resolve_infix_prule(parsetree, in_sym, in_fn_sym, LEFT_BIND);
}

tuple_t colon_pr(object_t env, list_t parsetree) 
{
  // how about rightbinding?
  // a:(b:c)  would be casting c to type b and then to type a
  tuple_t pre_t = resolve_infix_prule(parsetree, colon_sym, colon_fn_sym, RIGHT_BIND);
  tuple_t t = tuple_new(3);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, tuple_get(pre_t, 1));
  tuple_set(t, 2, quoted(resolve(env, tuple_get(pre_t, 2))));
  return t;
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
  return resolve_infix_prule(parsetree, and_sym, and_fn_sym, LEFT_BIND);
}

tuple_t or_pr(object_t env, list_t parsetree) 
{ 
  return resolve_infix_prule(parsetree, or_sym, or_fn_sym, LEFT_BIND);
}

#define HANDLE_STUFF_LIKE_RETURN(ttt) list_popfirst(parsetree);	\
  tuple_t t = tuple_new(2);					\
  tuple_set(t, 0, WRAP_SYMBOL(ttt ## _fn_sym));			\
  if (list_len(parsetree) > 0)					\
    tuple_set(t, 1, WRAP_PTR(LIST_T, parsetree));		\
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
  tuple_t t = tuple_new(5);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  tuple_set(t, 2, tuple_get(pre_t, 1));
  tuple_set(t, 3, quoted(tuple_get(pre_t, 2)));
  tuple_set(t, 4, quoted(tuple_get(pre_t, 3)));
  return t;
}

tuple_t try_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule2(env, parsetree, try_fn_sym, try_sym, 1, catch_sym, 2);
  tuple_t t = tuple_new(5);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  tuple_set(t, 2, quoted(tuple_get(pre_t, 1)));
  object_t obj = tuple_get(pre_t, 2);
  if (ptype(obj) != SYMBOL_T)
    rha_error("in 'try 17 catch (x) 42', 'x' must be a symbol");
  tuple_set(t, 3, quoted(obj));
  tuple_set(t, 4, quoted(tuple_get(pre_t, 3)));
  return t;
}

tuple_t while_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, while_fn_sym,
					 while_sym, 2);
  tuple_t t = tuple_new(4);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  tuple_set(t, 2, quoted(tuple_get(pre_t, 1)));
  tuple_set(t, 3, quoted(tuple_get(pre_t, 2)));
  return t;
}

tuple_t for_pr(object_t env, list_t parsetree)
{ 
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, for_fn_sym, for_sym, 2);
  // the second argument (e.g. (x in l) must be splitted into two
  tuple_t t = tuple_new(5);
  tuple_set(t, 0, tuple_get(pre_t, 0));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  // check now whether the first arg has the right form!
  object_t obj = tuple_get(pre_t, 1);
  if (ptype(obj)!=TUPLE_T)
    rha_error("second arg to 'for' must look like (x in l)");
  tuple_t arg = UNWRAP_PTR(TUPLE_T, obj);
  if (tuple_len(arg) != 3)
    rha_error("second arg to 'for' must look like (x in l)");
  obj = tuple_get(arg, 0);
  if (ptype(obj) != SYMBOL_T)
    rha_error("second arg to 'for' must look like (x in l)");
  if (UNWRAP_SYMBOL(obj) != in_fn_sym)
    rha_error("second arg to 'for' must look like (x in l)");
  tuple_set(t, 2, quoted(tuple_get(arg, 1)));
  tuple_set(t, 3, tuple_get(arg, 2));
  tuple_set(t, 4, quoted(tuple_get(pre_t, 2)));
  return t;
}

tuple_t fn_pr(object_t env, list_t parsetree)
{ 
  // currently this functions contains mainly a dirty hack!
  tuple_t pre_t = resolve_freefix_prule1(env, parsetree, fn_fn_sym, fn_sym, 2);
  tuple_t t = tuple_new(4);
  tuple_set(t, 0, WRAP_SYMBOL(fn_fn_sym));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));
  if (tuple_len(pre_t) == 2) {
    // the case: fn () code
    tuple_set(t, 2, quoted(WRAP_PTR(TUPLE_T, tuple_new(0))));
    tuple_set(t, 3, quoted(tuple_get(pre_t, 1)));
  }
  else {
    assert(tuple_len(pre_t) == 3);
    object_t obj = tuple_get(pre_t, 1);
    if (ptype(obj) == TUPLE_T)
      // fn (x, y) code; fn (x, y, z) code; etc
      tuple_set(t, 2, quoted(obj));
    else
      // fn (x) code
      tuple_set(t, 2, quoted(WRAP_PTR(TUPLE_T, tuple_make(1, obj))));
    // add the code
    tuple_set(t, 3, quoted(tuple_get(pre_t, 2)));
  }
  //debug("(fn_pr) %o\n", WRAP_PTR(TUPLE_T, t));
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
  if (list_len(parsetree) > 0)					
    tuple_set(t, 1, WRAP_PTR(LIST_T, parsetree));	
  else								
    tuple_set(t, 1, 0);
  return t;
}








/**********************************************************************
 * 
 * Helper functions for prules
 *
 **********************************************************************/

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


tuple_t resolve_incdec_prule(list_t parsetree)
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
      tuple_set(t, 1, WRAP_PTR(LIST_T, parsetree));
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
      tuple_set(t, 1, WRAP_PTR(LIST_T, parsetree));
      return t;
    }
  }
  // else
  rha_error(" ++ and -- are only prefix/praefix operators");
  assert(1==0);
  return 0; // make gcc happy
}


tuple_t resolve_infix_prule(list_t parsetree, symbol_t prule_sym, symbol_t fun_sym, bool_t left_binding)
{
  glist_t sym_list;
  glist_init(&sym_list);
  glist_append(&sym_list, prule_sym);
  return resolve_infix_prule_list(parsetree, &sym_list, fun_sym, left_binding);
}


tuple_t resolve_infix_prule_list(list_t parsetree, glist_t *prule_sym_list, symbol_t fun_sym, bool_t left_binding)
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
  tuple_set(t, 1, WRAP_PTR(LIST_T, lhs));
  tuple_set(t, 2, WRAP_PTR(LIST_T, rhs));
  return t;
}

tuple_t resolve_assign_prule(object_t env, list_t parsetree, symbol_t prule_sym, glist_t *assign_sym_list)
// resolves the right-most assignment (could be = += -= *= /=)
{
  //  debug("resolve_assign_prule(%o)\n", WRAP_PTR(LIST_T, parsetree));

  // (0) let's find the first appearance (left-most) of an assignment
  tuple_t pre_t = resolve_infix_prule_list(parsetree, assign_sym_list,
					   assign_sym, RIGHT_BIND);
  // now we got something like (= x 18)
  // however, we need (= local x 18)
  // or from (= (x . a) 18) we need (= x a 18)
  tuple_t t = tuple_new(4);
  tuple_set(t, 0, WRAP_SYMBOL(assign_sym));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));

  // (1) let's first deal with the LHS
  // do we have a dot-expression on the LHS?
  object_t lhs_obj = tuple_get(pre_t, 1);
  object_t lhs_sym_obj = lhs_obj;
  if (ptype(lhs_obj) == LIST_T) {
    // note the shallow list copy, which is necessary since we
    // possibly need the original 'lhs' list for stuff like '+='
    list_t lhs = UNWRAP_PTR(LIST_T, copy_expr(lhs_obj));
    assert(list_len(lhs) > 0); // this has been checked in 'resolve_infix_prule'
    // the dot must be the second last in that list,
    // since we do not allow anything else than symbols as the last
    lhs_sym_obj = list_poplast(lhs);
    if (ptype(lhs_sym_obj) != SYMBOL_T)
      rha_error("(parsing) the assign sign must be preceeded by a symbol");
    if (list_len(lhs) > 0) {
      object_t a_dot = list_poplast(lhs);
      // this must be a "DOT"
      if ((ptype(a_dot) != SYMBOL_T) || (UNWRAP_SYMBOL(a_dot)!=dot_sym))
	rha_error("(parsing) preceeding the symbol must be a dot, "
		  "found '%o'", a_dot);
      // now, 'lhs' contains the LHS of the dot
      tuple_set(t, 1, WRAP_PTR(LIST_T, lhs));
    }
  }
  else if (ptype(lhs_obj) != SYMBOL_T)
    rha_error("(parsing) can only assign to symbols");
  // quote the symbol:
  tuple_set(t, 2, quoted(lhs_sym_obj));
  // (2) to build the RHS we check whether the 'prule_sym' was
  // something else than 'equal_sym'
  object_t rhs_obj = resolve(env, tuple_get(pre_t, 2));
  if (prule_sym != equal_sym) {
    assert((prule_sym==plusequal_sym) 
	   || (prule_sym==minusequal_sym)
	   || (prule_sym==timesequal_sym)
	   || (prule_sym==divideequal_sym));
    tuple_t rhs = tuple_new(3);
    tuple_set(rhs, 1, resolve(env, lhs_obj));
    tuple_set(rhs, 2, resolve(env, rhs_obj));
    if (prule_sym==plusequal_sym) 
      tuple_set(rhs, 0, WRAP_SYMBOL(plus_fn_sym));
    else if (prule_sym==minusequal_sym) 
      tuple_set(rhs, 0, WRAP_SYMBOL(minus_fn_sym));
    else if (prule_sym==timesequal_sym) 
      tuple_set(rhs, 0, WRAP_SYMBOL(times_fn_sym));
    else if (prule_sym==divideequal_sym) 
      tuple_set(rhs, 0, WRAP_SYMBOL(divide_fn_sym));
    else
      assert(1==0);
    rhs_obj = WRAP_PTR(TUPLE_T, rhs);
  }
  // finally add it to the tuple
  tuple_set(t, 3, rhs_obj);
  //debug("resolve_assign_prule returns: %o\n", WRAP_PTR(TUPLE_T, 0, t));
  return t;
}

object_t get_cmp_fn(symbol_t cmp_s)
{
  if (cmp_s==equalequal_sym)        return WRAP_SYMBOL(equalequal_fn_sym);
  else if (cmp_s==equalequal_sym)   return WRAP_SYMBOL(notequal_fn_sym);
  else if (cmp_s==less_sym)         return WRAP_SYMBOL(less_fn_sym);
  else if (cmp_s==lessequal_sym)    return WRAP_SYMBOL(lessequal_fn_sym);
  else if (cmp_s==greater_sym)      return WRAP_SYMBOL(greater_fn_sym);
  else if (cmp_s==greaterequal_sym) return WRAP_SYMBOL(greaterequal_fn_sym);
  else rha_error("unkown cmp symbol");
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
	list_append(pieces, resolve_list_by_prules(env, parts));
	parts = list_new();
	continue;
      }
    }
    list_append(parts, head);
  }
  if (list_len(parts)==0)
    rha_error("cmp symbols must not be at the end of expression");
  list_append(pieces, resolve_list_by_prules(env, parts));

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

list_t remove_rounded_brackets(object_t expr)
{
  if (ptype(expr) != LIST_T)
    rha_error("(parsing) first args of freefix form must be enclosed"
	      "in round brackets, found %o", expr);
  list_t l = UNWRAP_PTR(LIST_T, expr);
  assert(list_len(l)>0); // otherwise parser problem
  object_t head = list_popfirst(l);
  assert(ptype(head) == SYMBOL_T);  // otherwise parser problem
  if (UNWRAP_SYMBOL(head) != rounded_sym)
    rha_error("(parsing) first args of freefix form must be enclosed"
	      "in round brackets, found %o", expr);
  return l;
}


list_t read_freefix_args(object_t env, list_t parsetree, int_t narg)
{
  if (list_len(parsetree) < narg)
    rha_error("(parsing) not enough arguments for freefix form");
  list_t args = list_new();
  for (int i=1; i<narg; i++) {
    object_t obj = list_popfirst(parsetree);
    // check its roundedness, extract its content and resolve
    obj = resolve_tuple(env, remove_rounded_brackets(obj));
    // we need to remove the 'rounded_sym' by hand...
    assert(ptype(obj)==TUPLE_T);
    tuple_t t = UNWRAP_PTR(TUPLE_T, obj);
    list_t l = tuple_to_list(t);
    list_popfirst(l);
    if (list_len(l) > 1)
      list_append(args, WRAP_PTR(TUPLE_T, list_to_tuple(l)));
    else if (list_len(l) == 1)
      list_append(args, list_popfirst(l));
    // else
    //   do nothing
  }
  // last argument is the rest of the list
  list_append(args, resolve_list_by_prules(env, parsetree));
  //debug("%o\n", WRAP_PTR(LIST_T, args));
  return args;
}

tuple_t resolve_freefix_prule1(object_t env, list_t parsetree,
			       symbol_t fun_sym,
			       symbol_t key1, int_t narg1)
{
  // resolve_freefix_prule(env, pt, for_fn_sym, for_sym, 2);
  // for (x in l) { ... }
  // resolve_freefix_prule(env, pt, while_fn_sym, while_sym, 2);
  // while (x>0) f(x)
  //
  // note that all but the last of several arguments to keys must be
  // in rounded brackets
  //debug("resolve_freefix_prule1(%o)\n", WRAP_PTR(LIST_T, parsetree));
  object_t head = list_popfirst(parsetree);
  assert(UNWRAP_SYMBOL(head) == key1);
  list_t call = read_freefix_args(env, parsetree, narg1);
  list_prepend(call, WRAP_SYMBOL(fun_sym));
  return list_to_tuple(call);
}


tuple_t resolve_freefix_prule2(object_t env, list_t parsetree,
			       symbol_t fun_sym,
			       symbol_t key1, int_t narg1, 
			       symbol_t key2, int_t narg2)
{
  // resolve_freefix_prule(env, pt, if_fn_sym, if_sym, 2, else_sym, 1);
  // if (x==0) 17
  // if (x==0) 17 else 42
  // resolve_freefix_prule(env, pt, try_fn_sym, try_sym, 1, catch_sym, 2);
  // try f(x)
  // try f(x) catch(excp) { }
  //
  // note that the expression with key2 is optional
  // note that all but the last of several arguments to keys must be
  // in rounded brackets
  //  debug("resolve_freefix_prule2(%o)\n", WRAP_PTR(LIST_T, parsetree));
  object_t head = list_popfirst(parsetree);
  assert(UNWRAP_SYMBOL(head) == key1);
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


