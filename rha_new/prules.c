#include "prules.h"

#include "core.h"
#include "list_fn.h"
#include "tuple_fn.h"
#include "messages.h"
#include "utils.h"
#include "assert.h"

// prules take a white-space list and return a function call!
// prules must not return a macro call!

// the prule symbols as they appear in Rhabarber
symbol_t plusplus_sym = 0;
symbol_t minusminus_sym = 0;
symbol_t not_sym = 0;
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
symbol_t run_sym = 0;
symbol_t if_sym = 0;
symbol_t try_sym = 0;
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

#define MAKE_PRULES(ttt, prio) f = lookup(module, ttt ## _pr_sym);	\
  assign(f, priority_sym, WRAP_REAL(prio));				\
  assign(module, ttt ## _sym, f);


void prules_init(object_t root, object_t module)
{
  // initialize prule symbols
  // the string must show the form that appears in Rhabarber
  // and can be different from the function name in C
  plusplus_sym     = symbol_new("++");
  minusminus_sym   = symbol_new("--");
  not_sym          = symbol_new("!");
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
  run_sym          = symbol_new("run");
  if_sym           = symbol_new("if");
  try_sym          = symbol_new("try");
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

  // add priority slots for all prules
  // and add those prules also as symbols
  // the strongest binding is the one with the smallest number!
  // note that 'dot' is the strongest binding and it is not a prule!
  // the second strongest is function call which are not a prule
  // either.  Both 'dots' and 'fncall' are dealt with by hand in
  // 'resolve_list_by_prule'
  object_t f = 0;
  MAKE_PRULES(plusplus, 1.0);   // increment, decrement
  MAKE_PRULES(minusminus, 1.0);
  MAKE_PRULES(not, 2.5);        // logics
  MAKE_PRULES(divide, 3.0);     // arithemics
  MAKE_PRULES(times, 4.0);
  MAKE_PRULES(minus, 5.0);
  MAKE_PRULES(plus, 6.0);
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
  MAKE_PRULES(run, 10.0);
  MAKE_PRULES(if, 10.0);
  MAKE_PRULES(try, 10.0);
  MAKE_PRULES(while, 10.0);
  MAKE_PRULES(for, 10.0);
  MAKE_PRULES(fn, 10.0);
  MAKE_PRULES(equal, 11.0);     // assignments
  MAKE_PRULES(plusequal, 11.0);
  MAKE_PRULES(minusequal, 11.0);
  MAKE_PRULES(timesequal, 11.0);
  MAKE_PRULES(divideequal, 11.0);
  MAKE_PRULES(quote, 15.0);     // quote
}


// forward declaration of helper functions
tuple_t resolve_prefix_prule(list_t parsetree, symbol_t fun_sym);
tuple_t resolve_infix_prule(list_t parsetree, symbol_t prule_sym, 
			    symbol_t fun_sym, bool_t left_binding);
tuple_t resolve_infix_prule_list(list_t parsetree, 
				 glist_t *prule_sym_list, 
				 symbol_t fun_sym, bool_t left_binding);
tuple_t resolve_assign_prule(list_t parsetree, symbol_t prule_sym, 
			     glist_t *assign_sym_list);


// all prules defined in C
// here they should end on '_pr'
//
// the reason is to allow some prule-names in C to be different from
// their name in rhabarber.  this is important for, e.g.:
//
//    RHA-NAME C-NAME     C-IMPLEMENTATION
//    *        times_pr   int_times
//    /        divide_pr  int_divide
//    return   return_pr  return_fn
//
// the keyword case is the reason why '_pr' is necessary
// also 
tuple_t plusplus_pr(list_t parsetree) { return tuple_new(0); }
tuple_t minusminus_pr(list_t parsetree) { return tuple_new(0); }
tuple_t not_pr(list_t parsetree) { return tuple_new(0); }
tuple_t divide_pr(list_t parsetree) {
  return resolve_infix_prule(parsetree, divide_sym, int_divide_sym, LEFT_BIND);
}
tuple_t times_pr(list_t parsetree) {
  return resolve_infix_prule(parsetree, times_sym, int_times_sym, LEFT_BIND);
}
tuple_t minus_pr(list_t parsetree) {
  return resolve_infix_prule(parsetree, minus_sym, int_minus_sym, LEFT_BIND);
}
tuple_t plus_pr(list_t parsetree) {
  return resolve_infix_prule(parsetree, plus_sym, int_plus_sym, LEFT_BIND);
}
tuple_t less_pr(list_t parsetree) { return tuple_new(0); }
tuple_t lessequal_pr(list_t parsetree) { return tuple_new(0); }
tuple_t greater_pr(list_t parsetree) { return tuple_new(0); }
tuple_t greaterequal_pr(list_t parsetree) { return tuple_new(0); }
tuple_t equalequal_pr(list_t parsetree) { return tuple_new(0); }
tuple_t notequal_pr(list_t parsetree) { return tuple_new(0); }
tuple_t and_pr(list_t parsetree) { return tuple_new(0); }
tuple_t or_pr(list_t parsetree) { return tuple_new(0); }
tuple_t return_pr(list_t parsetree) { return tuple_new(0); }
tuple_t deliver_pr(list_t parsetree) { return tuple_new(0); }
tuple_t break_pr(list_t parsetree) { return tuple_new(0); }
tuple_t throw_pr(list_t parsetree) { return tuple_new(0); }
tuple_t run_pr(list_t parsetree) { return tuple_new(0); }
tuple_t if_pr(list_t parsetree) { return tuple_new(0); }
tuple_t try_pr(list_t parsetree) { return tuple_new(0); }
tuple_t while_pr(list_t parsetree) { return tuple_new(0); }
tuple_t for_pr(list_t parsetree) { return tuple_new(0); }
tuple_t fn_pr(list_t parsetree) { return tuple_new(0); }
tuple_t equal_pr(list_t parsetree) {
  return resolve_assign_prule(parsetree, equal_sym, &assign_sym_list);
}
tuple_t plusequal_pr(list_t parsetree) {
  return resolve_assign_prule(parsetree, plusequal_sym, &assign_sym_list);
}
tuple_t minusequal_pr(list_t parsetree) {
  return resolve_assign_prule(parsetree, minusequal_sym, &assign_sym_list);
}
tuple_t timesequal_pr(list_t parsetree) {
  return resolve_assign_prule(parsetree, timesequal_sym, &assign_sym_list);
}
tuple_t divideequal_pr(list_t parsetree) {
  return resolve_assign_prule(parsetree, divideequal_sym, &assign_sym_list);
}
tuple_t quote_pr(list_t parsetree) {
  return resolve_prefix_prule(parsetree, quote_sym);
}








/**********************************************************************
 * 
 * Helper functions for prules
 *
 **********************************************************************/



tuple_t resolve_prefix_prule(list_t parsetree, symbol_t fun_sym)
// fs  the function symbol (of the function to be called)
// which is also the prule symbol
// e.g. \(17+42)    for the backslash
{
  // the prule symbol must be the first in the parsetree tuple
  int llen = list_len(parsetree);
  if (llen == 1) {
    // e.g. return or break (to finish a function, returning nothing)
    return tuple_make(1, WRAP_SYMBOL(fun_sym));
  }
  else if (llen > 1) {
    // e.g. return 17
    // chop off the prefix symbol
    list_popfirst(parsetree);
    tuple_t t = tuple_new(2);
    tuple_set(t, 0, WRAP_SYMBOL(fun_sym));
    tuple_set(t, 1, WRAP_PTR(LIST_T, list_proto, parsetree));
    return t;
  }
  else
    rha_error("resolve_prefix_prule: parse error.\n");
  // never reach this point
  assert(1==0);
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
    rha_error("resolve_infix_prule: infix requires a nonempty lhs and rhs.");
  tuple_t t = tuple_new(3);
  tuple_set(t, 0, WRAP_SYMBOL(fun_sym));
  tuple_set(t, 1, WRAP_PTR(LIST_T, list_proto, lhs));
  tuple_set(t, 2, WRAP_PTR(LIST_T, list_proto, rhs));
  return t;
}

tuple_t resolve_assign_prule(list_t parsetree, symbol_t prule_sym, glist_t *assign_sym_list)
// resolves the right-most assignment (could be = += -= *= /=)
{
  // (0) let's find the first appearance (left-most) of an assignment
  tuple_t pre_t = resolve_infix_prule_list(parsetree, assign_sym_list,
				       assign_sym, LEFT_BIND);
  // now we got something like (= x 18)
  // however, we need (= local x 18)
  // or from (= (x . a) 18) we need (= x a 18)
  tuple_t t = tuple_new(4);
  tuple_set(t, 0, WRAP_SYMBOL(assign_sym));
  tuple_set(t, 1, WRAP_SYMBOL(local_sym));

  // (1) let's first deal with the LHS
  // do we have a dot-expression on the LHS?
  object_t lhs_obj = tuple_get(pre_t, 1);
  if (ptype(lhs_obj) == LIST_T) {
    // note the shallow list copy, which is necessary since we
    // possibly need the original 'lhs' list for stuff like '+='
    list_t lhs = list_copy(UNWRAP_PTR(LIST_T, lhs_obj));
    assert(list_len(lhs) > 0); // this has been checked in 'resolve_infix_prule'
    // the dot must be the second last in that list
    // since we do not allow anything else than symbols as the last
    lhs_obj = list_poplast(lhs);
    if (ptype(lhs_obj) != SYMBOL_T)
      rha_error("(parsing) the assign sign must be preceeded by a symbol\n");
    if (list_len(lhs) > 0) {
      object_t a_dot = list_poplast(lhs);
      // this must be a "DOT"
      if ((ptype(a_dot) != SYMBOL_T) || (UNWRAP_SYMBOL(a_dot)!=dot_sym))
	rha_error("(parsing) preceeding the symbol must be a dot\n");
      // now, 'lhs' contains the LHS of the dot
      tuple_set(t, 1, WRAP_PTR(LIST_T, list_proto, lhs));
    }
  }
  else if (ptype(lhs_obj) != SYMBOL_T)
    rha_error("(parsing) can only assign to symbols.");
  // quote the symbol:
  // note that we create a list since this list will be resolved
  // somewhere else
  list_t quote_call = list_new();
  list_append(quote_call, WRAP_SYMBOL(quote_sym));
  list_append(quote_call, lhs_obj);
  tuple_set(t, 2, WRAP_PTR(LIST_T, list_proto, quote_call));
  // (2) to build the RHS we check whether the 'prule_sym' was
  // something else than 'equal_sym'
  object_t rhs_obj = tuple_get(pre_t, 2);
  if (prule_sym != equal_sym) {
    assert((prule_sym==plusequal_sym) 
	   || (prule_sym==minusequal_sym)
	   || (prule_sym==timesequal_sym)
	   || (prule_sym==divideequal_sym));
    tuple_t rhs = tuple_new(3);
    tuple_set(rhs, 0, lhs_obj);
    tuple_set(rhs, 2, tuple_get(pre_t, 2));
    if (prule_sym==plusequal_sym) 
      tuple_set(rhs, 1, WRAP_SYMBOL(plus_sym));
    else if (prule_sym==minusequal_sym) 
      tuple_set(rhs, 1, WRAP_SYMBOL(minus_sym));
    else if (prule_sym==timesequal_sym) 
      tuple_set(rhs, 1, WRAP_SYMBOL(times_sym));
    else if (prule_sym==divideequal_sym) 
      tuple_set(rhs, 1, WRAP_SYMBOL(divide_sym));
    else
      assert(1==0);
    rhs_obj = WRAP_PTR(TUPLE_T, tuple_proto, rhs);
  }
  // finally add it to the tuple
  tuple_set(t, 3, rhs_obj);
  return t;
}

#define IGNORE
#ifndef IGNORE

tuple_t precall(int narg, ...)
{
  va_list ap;
  tuple_t t = tuple_new(narg+1);
  va_start(ap, narg);
  for (int i = 0; i < narg; i++)
    tuple_set(t, i+1, va_arg(ap, object_t));
  va_end(ap);

  tuple_set(t, 0, WRAP_SYMBOL(rounded_sym));

  return t;
}


object_t precall1(object_t f, object_t arg1)
// this will create:
//     (f (rounded_sym arg1))
// this expression will be converted in a function call by resolve_prules
{
  return tuple_make(2, f, tuple_make(2, rounded_sym, arg1));
}


object_t precall2(object_t f, object_t arg1, object_t arg2)
// this will create:
//     (f (rounded arg1 arg2))
// this expression will be converted in a function call by resolve_prules
{
  return tuple_make(2, f, tuple_make(3, rounded_sym, arg1, arg2));
}

static list_tr cmp_list = 0; // a list of comparison prule symbols

BUILTIN(resolve_cmp_prule)  // the only arg we are using is 'in'
// resolve stuff like a == b == c < d
{
  if (!cmp_list) {
    // make a list of the comparison prule symbol
    cmp_list = list_new();
    list_append(cmp_list, less_sym);
    list_append(cmp_list, lessequal_sym);
    list_append(cmp_list, greater_sym);
    list_append(cmp_list, greaterequal_sym);
    list_append(cmp_list, equal_sym);
    list_append(cmp_list, notequal_sym);
  }
  // chop the parsetree into pieces
  assert(tuple_length(in)==3);
  tuple_tr parsetreetuple = tuple_get(in, 2);
  list_tr parsetree = tuple_to_list(parsetreetuple);
  debug("%o\n", parsetree);
  object_t lhs = list_chop_first_list(parsetree, cmp_list);
  object_t rhs = list_chop_first_list(parsetree, cmp_list);
  int op_pos = list_length(lhs);
  // add the 'op' string to the symbol
  symbol_tr old = tuple_get(parsetreetuple, op_pos);
  symbol_tr op = symbol_new(string_plus_string("op", symbol_name(old)));
  lhs = list_solidify(lhs);
  op_pos += 1 + list_length(rhs);
  rhs = list_solidify(rhs);
  object_t result = 0;
  if (list_length(parsetree)==0) {
    result = tuple_make(3, op, lhs, rhs);
  }
  else {
    // there are more cmp-operators to deal with
    result = precall2(op, lhs, rhs);
    do {
      old = tuple_get(parsetreetuple, op_pos);
      op = symbol_new(string_plus_string("op", symbol_name(old)));
      lhs = rhs;
      rhs = list_chop_first_list(parsetree, cmp_list);
      op_pos += 1 + list_length(rhs);
      rhs = list_solidify(rhs);
      if (list_length(parsetree)==0) {
	result = tuple_make(3, op_and_sym, result, precall2(op, lhs, rhs));
	break;
      }
      else {
	// accumulate
	result = precall2(op_and_sym, result, precall2(op, lhs, rhs));
      }
    } while (true);  // this loop is terminated by "break"
  }

  return result;
}


static list_tr incdec_list = 0; // a list with ++ and --

BUILTIN(resolve_incdec_prule)
{
  if (!incdec_list) {
    // make a list of the incdec prule symbols
    incdec_list = list_new();
    list_append(incdec_list, plusplus_sym);
    list_append(incdec_list, minusminus_sym);
  }
  CHECK_TYPE(tuple, in);
  assert(tuple_length(in) == 3);
  // let's discuss plusplus (minusminus is the same)
  // plusplus_sym must be at the first and/or the last position of the parsetree
  tuple_tr parsetree = tuple_get(in, 2);
  int tlen = tuple_length(parsetree);
  assert(tlen > 1);   // at least ++x or x++, but also ++x++
  list_tr rhs = tuple_to_list(parsetree);
  list_tr lhs = list_chop_first_list(rhs, incdec_list);
  int op_pos = list_length(lhs);
  symbol_tr op = 0;
  object_t result = 0;
  if (op_pos == 0) {
    // (1) at least one operator is at the first entry
    op = tuple_get(parsetree, op_pos);
    result = tuple_make(2, preplusplus_sym, list_solidify(rhs));
    if (symbol_equal_symbol(op, minusminus_sym)) tuple_set(result, 0, preminusminus_sym);
  }
  else if (op_pos == tlen-1) {
    // (2) a single operator at the last entry
    op = tuple_get(parsetree, op_pos);
    result = tuple_make(2, postplusplus_sym, list_solidify(lhs));
    if (symbol_equal_symbol(op, minusminus_sym)) tuple_set(result, 0, postminusminus_sym);
  }
  else {
    rha_error("resolve_incdec_prule: ++ or -- are only prefix/praefix operatoren");
  }
  return result;
}



/* object_t check_and_extract_condition(object_t cond) */
/* { */
/*   // the condition must be only one grouped element */
/*   if (HAS_TYPE(list, cond) && list_length(cond) == 1) */
/*     cond = list_solidify(cond); */
/*     if (!iscallof(group_sym, cond) || tuple_length(cond) != 2) */
/*       rha_error("The condition of 'if' must be one expr enclosed in brackets.\n"); */
/*     cond = tuple_get(cond, 1); // get rid of the brackets */
  
/* } */


object_t resolve_ctrl_prule(tuple_tr in)
// if (cond) code
// if (cond) code else code_else
// try code catch (x) code
// while (cond) code
// for (init, check, update) code
// for (x in y) code
// fn () code                // functions
// fn (x) code
// fn (x, y) code
// ...
// macro () code             // macro (like functions, but args are not evaluated)
// macro (x) code
// macro (x, y) code
// ...
// prule (priority, parsetree_var) code    // parse rule
{
  assert(tuple_length(in)==3);
  object_t t = tuple_get(in, 2);
  symbol_tr s = tuple_get(in, 1);
  assert(HAS_TYPE(symbol, s));
  CHECK_TYPE(tuple, t);
  list_tr code = tuple_to_list(t);
  // all above prules have at least three parts
  if (list_length(code) < 3) rha_error("resolve_ctrl_prule error 1.\n");
  object_t o = list_pop(code);
  // all above prules start with a keyword
  if (!HAS_TYPE(symbol, o) || !symbol_equal_symbol(o, s))
    rha_error("resolve_ctrl_prule error 2.\n");
  object_t cond = list_pop(code);
  
  // extra stuff for various symbols
  object_t result = 0;
  if (symbol_equal_symbol(s, if_sym)) {
    // if (cond) code_then
    // if (cond) code_then else code
    if (!iscallof(tuplefy_sym, cond) || tuple_length(cond) != 2)
      rha_error("Condition for 'if' must be a single expression in brackets.\n");
    cond = tuple_get(cond, 1);
    // check for else
    int codelen = list_length(code);
    list_tr code_then = list_chop_matching(code, if_sym, else_sym);
    if (codelen == list_length(code_then)+1)  // there was no code after "else"
      rha_error("'else' must be followed by code.\n");

    if (list_length(code) == 0)
      // no 'else'
      result = tuple_make(3, op_if_sym, cond, 
			  list_solidify(code_then));
    else
      // with 'else'
      result = tuple_make(4, op_if_sym, cond, 
			  list_solidify(code_then),
			  list_solidify(code));
  }
  else if (symbol_equal_symbol(s, try_sym)) {
    // try tryblock catch (catchvar) catchblock
    list_prepend(code, cond);   // there is no 'cond' part here, so put it back
    list_tr tryblock = list_chop_matching(code, try_sym, catch_sym);
    if (list_length(tryblock)==0)
      rha_error("try-block is missing.\n");
    int lcode = list_length(code);
    if (lcode == 0) 
      rha_error("catch-block is missing.\n");
    else if (lcode == 1) 
      rha_error("catch-block must have catch variable and catch-code.\n");
    object_t catchvar = list_pop(code);  // the catch variable
    if (!iscallof(tuplefy_sym, catchvar) || tuple_length(catchvar) != 2)
      rha_error("'catch' must be followed by a single expression (catch variable) in brackets.\n");
    catchvar = tuple_get(catchvar, 1);
    result = tuple_make(4, op_try_sym,
			list_solidify(tryblock),
			catchvar,
			list_solidify(code));
  }
  else if (symbol_equal_symbol(s, while_sym)) {
    // while (cond) code
    if (!iscallof(tuplefy_sym, cond) || tuple_length(cond) != 2)
      rha_error("Condition for 'while' must be a single expression in brackets.\n");
    cond = tuple_get(cond, 1);
    // that's it
    result = tuple_make(3, op_while_sym, cond, list_solidify(code));
  }
  else if (symbol_equal_symbol(s, for_sym)) {
    // for (init, check, update) code
    // for (element in container) code
    // note that 'cond' contains the parameters of 'for'
    if (iscallof(tuplefy_sym, cond) && tuple_length(cond)==4) {
      object_t init = tuple_get(cond, 1);
      object_t check = tuple_get(cond, 2);
      object_t update = tuple_get(cond, 3);
      result = tuple_make(5, op_for_cstyle_sym, init, check, update, 
			  list_solidify(code));
    }
    else if (iscallof(tuplefy_sym, cond) && tuple_length(cond)==2) {
      // extract element and container
      cond = tuple_get(cond, 1);
      if (tuple_length(cond)>=3) {
	list_tr condlist = tuple_to_list(cond);
	object_t t0 = list_pop(condlist);
	object_t t1 = list_pop(condlist);
	if (HAS_TYPE(symbol, t1) && symbol_equal_symbol(in_sym, t1))
	  result = tuple_make(4, op_for_iterator_sym, t0, 
			      list_solidify(condlist), list_solidify(code));
      }
    }
    if (!result) rha_error("'for' requires special forms for its arg.\n");
  }
  else if (symbol_equal_symbol(s, fn_sym) || symbol_equal_symbol(s, macro_sym)) {
    // fn () code
    // fn (x) code
    // fn (x, y) code
    // macro () code
    // macro (x) code
    // macro (x, y) code

    // note that 'cond' are for 'fn' and 'macro' their args
    // do some checks on the args
    if (!iscallof(tuplefy_sym, cond))
      rha_error("args must be a tuple of symbols or a single grouped symbol.\n");
    if (symbol_equal_symbol(s, fn_sym)) 
      tuple_set(cond, 0, op_fn_sym);
    else  
      tuple_set(cond, 0, op_macro_sym);
    result = tuple_to_list(cond);
    list_append(result, list_solidify(code));
    result = list_solidify(result);
  }
  else if (symbol_equal_symbol(s, prule_sym)) {
    // prule (priority, parsetree_var) code
    // note that 'cond' for 'prule' is a pair which contains a real number 
    // and a symbol with which we can access the parsetree
    if (iscallof(tuplefy_sym, cond)) {
      if (tuple_length(cond)!=3) 
	rha_error("prule has a special form: prule (priority, parsetree_var) code.\n");
    }
    else 
      rha_error("prule has a special form: prule (priority, parsetree_var) code.\n");
    tuple_set(cond, 0, op_prule_sym);
    result = tuple_to_list(cond);
    list_append(result, list_solidify(code));
    result = list_solidify(result);
  }

  if (!result) rha_error("resolve_crt_prule error.\n");
  return result;
}

object_t resolve_try_catch_prule(tuple_tr in) {
  return 0;
}

/*
  Program logic
*/ 


/*
 * lazy_or and lazy_and macro
 */

BUILTIN(b_lazy_or)
{
  int nin = tuple_length(in);
  if (nin == 3) {
    fprint(stdout, "lazy or\n");
    object_t lhs = tuple_get(in, 1);
    object_t rhs = tuple_get(in, 2);
    lhs = eval(env, lhs);
    if (HAS_TYPE(bool, lhs))
      if (bool_get(lhs))
	return bool_new(true);
      else {
	rhs = eval(env, rhs);
	if (HAS_TYPE(bool, rhs))
	  if (bool_get(rhs))
	    return bool_new(true);
	  else
	    return bool_new(false);
	else
	  rha_error("rhs of 'op||' must eval to bool.\n");
      }
    else
      rha_error("lhs of 'op||' must eval to bool.\n");
  }
  // otherwise wrong number of args
  rha_error("'op||' must be called with exactly two args.\n");
  return 0;
}

BUILTIN(b_lazy_and)
{
  int nin = tuple_length(in);
  if (nin == 3) {
    fprint(stdout, "lazy and\n");
    object_t lhs = tuple_get(in, 1);
    object_t rhs = tuple_get(in, 2);
    lhs = eval(env, lhs);
    if (HAS_TYPE(bool, lhs))
      if (!bool_get(lhs))
	return bool_new(false);
      else {
	rhs = eval(env, rhs);
	if (HAS_TYPE(bool, rhs))
	  if (bool_get(rhs))
	    return bool_new(true);
	  else
	    return bool_new(false);
	else
	  rha_error("rhs of 'op&&' must eval to bool.\n");
      }
    else
      rha_error("lhs of 'op&&' must eval to bool.\n");
  }
  // otherwise wrong number of args
  rha_error("'op&&' must be called with exactly two args.\n");
  return 0;
}


#endif
