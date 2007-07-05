/*
 * core - initialize classes and core builtin functions
 
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include <stdlib.h>
#include <setjmp.h>

#include "core.h"
#include "overloaded_tr.h"
#include "utils.h"

//#define DEBUG
#include "debug.h"

// forward declarations
#include <stdio.h>
#include <assert.h>
#include "messages.h"
#include "builtin_tr.h"
#include "plain_tr.h"
#include "bool_tr.h"
#include "int_tr.h"
#include "real_tr.h"
#include "complex_tr.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "object.h"
#include "prule.h"
#include "eval.h"
#include "function_tr.h"
#include "rhaparser_extra.h"
#include "matrix_tr.h"
#include "none_tr.h"
#include "symtable.h"

#define CORE_SYMBOLS_DEFINE
#include "core_symbols.h"
#undef CORE_SYMBOLS_DEFINE

/*
  Forward references
*/

// prules (parse rules) for symbolic operators
BUILTIN(b_plus);
BUILTIN(b_minus);
BUILTIN(b_times);
BUILTIN(b_divide);
BUILTIN(b_and);
BUILTIN(b_or);
BUILTIN(b_quote);
BUILTIN(b_dot);
//BUILTIN(b_nobinddot);
BUILTIN(b_quest);
BUILTIN(b_not);

// some generic prules, these function-names are lacking the 'b_'
// because they are not called the remainder
// (note: 'b_plus' is related to 'plus_sym')
BUILTIN(resolve_incdec_prule);  // for ++  --
BUILTIN(resolve_cmp_prule);     // for < <= > >= == !=
BUILTIN(resolve_assign_prule);  // for = += -= *= /=

// other functions with prules
BUILTIN(b_if);     // the prule for if
BUILTIN(b_op_if);  // the function for if
BUILTIN(b_fn);
BUILTIN(b_macro);
BUILTIN(b_prule);
BUILTIN(b_return);
BUILTIN(b_op_return);
BUILTIN(b_deliver);
BUILTIN(b_op_deliver);
BUILTIN(b_break);
BUILTIN(b_op_break);
BUILTIN(b_do);
BUILTIN(b_try);
BUILTIN(b_op_try);
BUILTIN(b_throw);
BUILTIN(b_op_throw);
BUILTIN(b_import); 
BUILTIN(b_op_import); 
BUILTIN(b_while);
BUILTIN(b_op_while);
BUILTIN(b_for);
BUILTIN(b_op_for_cstyle);
BUILTIN(b_op_for_iterator);

BUILTIN(b_preplusplus);      // function for prefix ++
BUILTIN(b_preminusminus);    // function for prefix --
BUILTIN(b_postplusplus);     // function for postfix ++
BUILTIN(b_postminusminus);   // function for postfix --

BUILTIN(b_lazy_and);
BUILTIN(b_lazy_or);

BUILTIN(b_eval);
BUILTIN(b_delete);
BUILTIN(b_exit);
BUILTIN(b_tuplefy);
BUILTIN(b_clone);
BUILTIN(b_load); 
BUILTIN(b_print);
BUILTIN(b_slots);
BUILTIN(b_allslots);
BUILTIN(b_addr);
BUILTIN(b_error);
BUILTIN(b_id);
BUILTIN(b_tic);
BUILTIN(b_toc);
BUILTIN(b_snatch);

/*
  Initializing the core environment stuff
*/
list_tr path;
object_t root; 
/* list_tr keywords = 0; */

void core_init(void)
{
  // nothing to do here because no new primtype is defined
  list_init();

  path = list_new();
}

object_t core_stub_init(void)
{
  // requires
  symbol_init();
  builtin_init();
  overloaded_init();
  bool_init();  // for lazy-or and lazy-and
  
#define CORE_SYMBOLS_INIT
#include "core_symbols.h"
#undef CORE_SYMBOLS_INIT

  // create the root object that holds all other objects
  root = plain_new();
  object_assign(root, root_sym, root);
  object_assign(root, symbol_new("path"), path);

  // add all symbols to the root
#define CORE_SYMBOLS_ADD
#include "core_symbols.h"
#undef CORE_SYMBOLS_ADD

  // attach basic builtin stuff to the object hierarchy
  ADDBUILTINMACRO(root, "op_if", b_op_if);
  ADDBUILTINMACRO(root, "op_while", b_op_while);
  ADDBUILTINMACRO(root, "op_for_iterator", b_op_for_iterator);
  ADDBUILTINMACRO(root, "op_for_cstyle", b_op_for_cstyle);
  ADDBUILTIN(root, "op_return", b_op_return);
  ADDBUILTIN(root, "op_deliver", b_op_deliver);
  ADDBUILTIN(root, "op_break", b_op_break);
  ADDBUILTIN(root, "op_throw", b_op_throw);
  ADDBUILTINMACRO(root, "do", b_do);
  ADDBUILTINMACRO(root, "op_try", b_op_try);
  ADDBUILTINMACRO(root, "op_import", b_op_import);
  ADDBUILTIN(root, "eval", b_eval);
  ADDBUILTIN(root, "delete", b_delete);
  ADDBUILTIN(root, "exit", b_exit);
  ADDBUILTIN(root, "tuplefy", b_tuplefy);
  ADDBUILTIN(root, "clone", b_clone);
  ADDBUILTIN(root, "load", b_load);
  ADDBUILTIN(root, "print", b_print);
  ADDBUILTIN(root, "slots", b_slots);
  ADDBUILTIN(root, "allslots", b_allslots);
  ADDBUILTIN(root, "addr", b_addr);
  ADDBUILTIN(root, "error", b_error);
  ADDBUILTIN(root, "id", b_id);
  ADDBUILTIN(root, "tic", b_tic);
  ADDBUILTIN(root, "toc", b_toc);
  ADDBUILTINMACRO(root, "preplusplus", b_preplusplus);
  ADDBUILTINMACRO(root, "preminusminus", b_preminusminus);
  ADDBUILTINMACRO(root, "postplusplus", b_postplusplus);
  ADDBUILTINMACRO(root, "postminusminus", b_postminusminus);
  ADDBUILTIN(root, "=?", b_snatch);

  // add unary operators as overloaded functions to root
  overloaded_tr ov;
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), string_new("from"));
  object_assign(root, from_sym, ov);

  // add binary operators as overloaded functions to root
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_dot_sym);
  object_assign(root, op_dot_sym, ov); 
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_plus_sym);
  object_assign(root, op_plus_sym, ov); 
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_minus_sym);
  object_assign(root, op_minus_sym, ov); // also unary
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_times_sym);
  object_assign(root, op_times_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_divide_sym);
  object_assign(root, op_divide_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_equal_sym);
  object_assign(root, op_equal_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), greater_sym);
  object_assign(root, op_greater_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_less_sym);
  object_assign(root, op_less_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), inc_sym);
  object_assign(root, inc_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), dec_sym);
  object_assign(root, dec_sym, ov);
  ov = overloaded_new();  object_assign(ov, symbol_new("name"), op_not_sym);
  object_assign(root, op_not_sym, ov);

  // add the lazy-or and lazy-and macros
  object_assign(root, op_and_sym, builtin_new_macro(&b_lazy_and));
  object_assign(root, op_or_sym, builtin_new_macro(&b_lazy_or));

  object_assign(root, symbol_new("modules"), plain_new());
/*   object_assign(root, symbol_new("keywords"), 0); */

  // maybe 'keywords' is not the best mechanism to avoid overwriting
  // of keywords, better would be to assign all basic symbol not to
  // 'root' but to 'root.basics', and all symbols in root.basics are
  // not easily overwritable...
/*   // collect all keywords */
/*   keywords = list_new(); */
/*   object_assign(root, symbol_new("keywords"), keywords); */
/*   list_append(keywords, quote_sym); */
/*   list_append(keywords, if_sym); */
/*   list_append(keywords, else_sym); */
/*   list_append(keywords, while_sym); */
/*   list_append(keywords, for_sym); */
/*   list_append(keywords, try_sym); */
/*   list_append(keywords, catch_sym); */
/*   list_append(keywords, throw_sym); */
/*   list_append(keywords, fn_sym); */
/*   list_append(keywords, return_sym); */
/*   list_append(keywords, deliver_sym); */
/*   list_append(keywords, break_sym); */
/*   list_append(keywords, import_sym); */
/*   list_append(keywords, prule_sym); */
/*   list_append(keywords, true_sym); */
/*   list_append(keywords, false_sym); */
/*   // many are missing here... */

  // add prules for all operators with the priorities
  // notice that plus and minus have different precedences
  // this to achieve:
  //     1-2+3   ->   (1-2)+3 == 2
  //     1+2-3   ->   1+(2-3) == 0
  // similar effect for times/divide:
  //     1/2*3   ->   (1/2)*3 == 1.5
  //     1*2/3   ->   1*(2/3) == 2/3

  // see http://en.wikipedia.org/wiki/Order_of_operations

  // prefix (strong binding) and postfix ++ --
  // the inc/dec operators are all resolved by the same prule:
  builtin_tr bnp = builtin_new_prule(&resolve_incdec_prule, 1.0);
  object_assign(root, plusplus_sym,       bnp);
  object_assign(root, minusminus_sym,     bnp);
  object_assign(root, not_sym,            builtin_new_prule(&b_not, 2.5));

  // infix (for terms)
  object_assign(root, dot_sym,            builtin_new_prule(&b_dot, 0.5));
  object_assign(root, quest_sym,          builtin_new_prule(&b_quest, 2.0));
  object_assign(root, divide_sym,         builtin_new_prule(&b_divide, 3.0));
  object_assign(root, times_sym,          builtin_new_prule(&b_times, 4.0));
  object_assign(root, minus_sym,          builtin_new_prule(&b_minus, 5.0));
  object_assign(root, plus_sym,           builtin_new_prule(&b_plus, 6.0));
  // the comparison symbols are all resolved by the same prule:
  bnp = builtin_new_prule(&resolve_cmp_prule, 7.0);
  object_assign(root, less_sym,           bnp);
  object_assign(root, lessequal_sym,      bnp);
  object_assign(root, greater_sym,        bnp);
  object_assign(root, greaterequal_sym,   bnp);
  object_assign(root, equal_sym,          bnp);
  object_assign(root, notequal_sym,       bnp);
  object_assign(root, and_sym,            builtin_new_prule(&b_and, 8.0));
  object_assign(root, or_sym,             builtin_new_prule(&b_or, 9.0));
  // the assignment symbols are all resolved by same prule:
  bnp = builtin_new_prule(&resolve_assign_prule, 10.0);
  object_assign(root, assign_sym,         bnp);
  object_assign(root, plusassign_sym,     bnp);
  object_assign(root, minusassign_sym,    bnp);
  object_assign(root, timesassign_sym,    bnp);
  object_assign(root, divideassign_sym,   bnp);

  // prefix (loosely binding), similar to statements
  object_assign(root, quote_sym,          builtin_new_prule(&b_quote, 10.0));
  object_assign(root, return_sym,         builtin_new_prule(&b_return, 10.0));
  object_assign(root, deliver_sym,        builtin_new_prule(&b_deliver, 10.0));
  object_assign(root, break_sym,          builtin_new_prule(&b_break, 10.0));
  object_assign(root, throw_sym,          builtin_new_prule(&b_throw, 10.0));
  object_assign(root, import_sym,         builtin_new_prule(&b_import, 10.0));
  object_assign(root, if_sym,             builtin_new_prule(&b_if, 10.0));
  object_assign(root, try_sym,            builtin_new_prule(&b_try, 10.0));
  object_assign(root, while_sym,          builtin_new_prule(&b_while, 10.0));
  object_assign(root, for_sym,            builtin_new_prule(&b_for, 10.0));
  object_assign(root, fn_sym,             builtin_new_prule(&b_fn, 10.0));
  object_assign(root, macro_sym,          builtin_new_prule(&b_macro, 10.0));
  object_assign(root, prule_sym,          builtin_new_prule(&b_prule, 10.0));

  return root;
}


/************************************************************

Builtin functions

************************************************************/

/*
  Prule stuff
*/

tuple_tr precall(int narg, ...);
object_t precall1(object_t f, object_t arg1);
object_t precall2(object_t f, object_t arg1, object_t arg2);
object_t resolve_prefix_prule(tuple_tr in, symbol_tr fs);
object_t resolve_infix_prule(tuple_tr in, symbol_tr fs, bool left_binding);
object_t resolve_ctrl_prule(tuple_tr in);

#define LEFT_BIND true
#define RIGHT_BIND false
BUILTIN(b_dot) { return resolve_infix_prule(in, op_dot_sym, LEFT_BIND); }
BUILTIN(b_quest) { return resolve_infix_prule(in, op_quest_sym, LEFT_BIND); }
BUILTIN(b_plus) { return resolve_infix_prule(in, op_plus_sym, LEFT_BIND); }
BUILTIN(b_minus) { 
  // here we also have to take care of the prefix minus, 
  // which actually has a higher priority (== lower precedence)

  // suppose we are trying to resolve a minus-sign since it had higher
  // priority than a times sign, however, actually, the minus-sign is
  // a prefix minus and thus should have lower priority

  // (1) how can we identify that case?
  // look whether it is the left-most tuple-entry
  // if so, we treat it as a prefix-minus
  printdebug("b_minus in: %o\n", in);
  int nin = tuple_length(in);
  assert(nin == 3);
  symbol_tr ms = tuple_get(in, 1);  // the symbol that triggered this prule (here usually minus_sym)
  object_t parsetree = tuple_get(in, 2);
  assert(HAS_TYPE(symbol, ms));
  if (iscallof(ms, parsetree)) {
    // prefix minus:
    list_tr l = tuple_to_list(parsetree);
    // remove the first entry
    list_pop(l);
    // take the next element 
    object_t pm = precall1(op_minus_sym, list_pop(l));
    // add the new pair at the beginning
    list_prepend(l, pm);
    // and finally call resolve_prules on the whole tuple again...
    object_t result = list_solidify(l);
    result = tuple_make(2, id_sym, result);
    printdebug("b_minus result: %o\n", result);
    return result;
  }
  // otherwise we have to deal with an infix-minus the usual way
  else return resolve_infix_prule(in, op_minus_sym, LEFT_BIND); 
}
BUILTIN(b_times)        { return resolve_infix_prule(in, op_times_sym, LEFT_BIND); }
BUILTIN(b_divide)       { return resolve_infix_prule(in, op_divide_sym, LEFT_BIND); }
BUILTIN(b_and)          { return resolve_infix_prule(in, op_and_sym, RIGHT_BIND); }
BUILTIN(b_or)           { return resolve_infix_prule(in, op_or_sym, RIGHT_BIND); }

BUILTIN(b_quote)   { return resolve_prefix_prule(in, op_quote_sym); }
BUILTIN(b_not)     { 
  object_t result = resolve_prefix_prule(in, op_not_sym); 
  assert(HAS_TYPE(tuple, result));
  // 'not' doesn't allow stuff like:   (!, return, 17)  or  (!)
  int len = tuple_length(result);
  if ((len==1) || len>2) rha_error("'not' must have exactly one argument.\n");
  return result;
}
BUILTIN(b_return)  { return resolve_prefix_prule(in, op_return_sym); }
BUILTIN(b_deliver) { return resolve_prefix_prule(in, op_deliver_sym); }
BUILTIN(b_break)   { return resolve_prefix_prule(in, op_break_sym); }
BUILTIN(b_throw)   { return resolve_prefix_prule(in, op_throw_sym); }
BUILTIN(b_import)  { return resolve_prefix_prule(in, op_import_sym); }

BUILTIN(b_if)    { return resolve_ctrl_prule(in); }
BUILTIN(b_while) { return resolve_ctrl_prule(in); }
BUILTIN(b_for)   { return resolve_ctrl_prule(in); }
BUILTIN(b_fn)    { return resolve_ctrl_prule(in); }
BUILTIN(b_macro) { return resolve_ctrl_prule(in); }
BUILTIN(b_prule) { return resolve_ctrl_prule(in); }
BUILTIN(b_try)   { return resolve_ctrl_prule(in); }


/* 
   tools for programming prules
*/

tuple_tr precall(int narg, ...)
{
  va_list ap;
  tuple_tr t = tuple_new(narg+1);
  va_start(ap, narg);
  for (int i = 0; i < narg; i++)
    tuple_set(t, i+1, va_arg(ap, object_t));
  va_end(ap);

  tuple_set(t, 0, tuplefy_sym);

  return t;
}


object_t precall1(object_t f, object_t arg1)
// this will create:
//     (f (tuplefy_sym arg1))
// this expression will be converted in a function call by resolve_prules
{
  return tuple_make(2, f, tuple_make(2, tuplefy_sym, arg1));
}


object_t precall2(object_t f, object_t arg1, object_t arg2)
// this will create:
//     (f (tuplefy arg1 arg2))
// this expression will be converted in a function call by resolve_prules
{
  return tuple_make(2, f, tuple_make(3, tuplefy_sym, arg1, arg2));
}


/* symbol_tr remove_m(symbol_tr s) { */
/*   // by adding 2 to the string we all but the first two characters, */
/*   // e.g. "op+" becomes "op+" */
/*   return symbol_new(symbol_to_string(s) + 2); */
/* } */


object_t resolve_prefix_prule(tuple_tr in, symbol_tr fs)
// ms   the prule symbol
// fs   the function symbol (of the function to be called)
// e.g. \(17+42)    for the backslash
{
  assert(tuple_length(in)==3);  // the only arg is the parsetree
  // the prule symbol must be the first in the parsetree tuple
  symbol_tr ms = tuple_get(in, 1);  // the symbol of the calling prule
  assert(HAS_TYPE(symbol, ms));
  tuple_tr t = tuple_get(in, 2);    // the parsetree
  int tlen = tuple_length(t);
  assert(tlen > 0);  // because of the calling convention (see e.g. prule_resolve)
  object_t t0 = tuple_get(t, 0);  // must be the prule symbol
  if (!HAS_TYPE(symbol, t0))
    rha_error("resolve_prefix_prule: first element must be symbol.\n");
  if (!symbol_equal_symbol(ms, t0))
    rha_error("resolve_prefix_prule: first element must be prule symbol.\n");
  if (tlen == 1) {
    // e.g. return or break (to finish a function, returning nothing)
    return tuple_make(1, fs);
  }
  else if (tlen > 1) {
    // e.g. return 17
    // remove the first element and create the call to e.g. quote-function
    t = tuple_shiftfirst(t);
    if (tuple_length(t)==1) 
      t = tuple_get(t, 0);
    return tuple_make(2, fs, t);
  }
  else
    rha_error("resolve_prefix_prule: parse error.\n");

  assert(1==0);
}


object_t resolve_infix_prule(tuple_tr in, symbol_tr fs, bool left_binding)
// a + (b + c)
{
  // a generic infix prule which can deal with all infix operators
  printdebug("resolve_infix_prule in: %o\n", in);
  assert(tuple_length(in)==3);  // the only arg is the parsetree
  symbol_tr ms = tuple_get(in, 1);
  assert(HAS_TYPE(symbol, ms));
  // let's manipulate the parsetree
  list_tr lhs, rhs;
  if (left_binding) {
    lhs = tuple_to_list(tuple_get(in, 2));
    rhs = list_chop_last(lhs, ms);
  }
  else {
    rhs = tuple_to_list(tuple_get(in, 2));
    lhs = list_chop_first(rhs, ms);
  }

  if ((list_length(lhs)==0) || (list_length(rhs)==0))
    rha_error("resolve_infix_prule: infix requires a nonempty lhs and rhs.");
  printdebug("resolve_infix_prule result (lhs): %o\n", lhs);
  printdebug("resolve_infix_prule result (rhs): %o\n", rhs);
  tuple_tr result = tuple_new(3);
  tuple_set(result, 0, fs);
  tuple_set(result, 1, list_solidify(lhs));
  tuple_set(result, 2, list_solidify(rhs));

  printdebug("resolve_infix_prule result: %o\n", result);
  return result;
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
  printdebug("%o\n", parsetree);
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

static list_tr assign_list = 0; // a list with = += -= *= /=

BUILTIN(resolve_assign_prule)
// resolves the right-most assignment (could be = += -= *= /=)
{
  if (!assign_list) {
    // make a list of the assign prule symbols
    assign_list = list_new();
    list_append(assign_list, assign_sym);
    list_append(assign_list, plusassign_sym);
    list_append(assign_list, minusassign_sym);
    list_append(assign_list, timesassign_sym);
    list_append(assign_list, divideassign_sym);
  }
  CHECK_TYPE(tuple, in);
  assert(tuple_length(in) == 3);
  // let's find the first appearance (left-most) of an assignment
  tuple_tr t = tuple_get(in, 2);
  int tlen = tuple_length(t);
  for (int i=0; i<tlen; i++) {
    object_t obj = tuple_get(t, i);
    if (HAS_TYPE(symbol, obj)) {
      int j = 0;
      symbol_tr s = 0;
      list_foreach(s, assign_list) {
	j++;
	if (symbol_equal_symbol(s, obj)) {
	  // note that we always put the op_assign_sym as the function to call
	  tuple_set(in, 1, s);
	  object_t result = resolve_infix_prule(in, op_assign_sym, RIGHT_BIND);
	  switch (j) {
	  case 1: s = 0; break;  // plain assign
	  case 2: s = op_plus_sym; break;
	  case 3: s = op_minus_sym; break;
	  case 4: s = op_times_sym; break;
	  case 5: s = op_divide_sym; break;
	  }
	  if (s) 
	    tuple_set(result, 2, precall2(s, tuple_get(result, 1),
					  tuple_get(result, 2)));
	  // note that assign has three inputs:
	  //   scope, symbol, value
	  result = tuple_make(4, tuple_get(result, 0),
			      local_sym, 
			      tuple_get(result, 1),
			      tuple_get(result, 2));
	  // note that stuff like:
	  //     a.b = 17
	  // becomes
          //     assign_fn(a, b, 17)
	  // find the right most dot on the LHS
	  object_t LHS = tuple_get(result, 2);
	  if (HAS_TYPE(tuple, LHS)) {
	    int LHSlen = tuple_length(LHS);
	    for (int j=LHSlen-1; j>=0; j--) {
	      object_t LHSj = tuple_get(LHS, j);
	      if (HAS_TYPE(symbol, LHSj) 
		  && symbol_equal_symbol(LHSj, dot_sym))
		{
		  // split the tuple into two
		  tuple_tr scope = tuple_new(j);
		  tuple_tr var = tuple_new(LHSlen-j-1);
		  for (int k=0; k<j; k++)
		    tuple_set(scope, k, tuple_get(LHS, k));
		  for (int k=j+1; k<LHSlen; k++)
		    tuple_set(var, k-j-1, tuple_get(LHS, k));
		  // replace the default scope and the var
		  tuple_set(result, 1, scope);
		  tuple_set(result, 2, var);
		  break;
		}
	    }
	  }
	  return result;
	}
      }
    }
  }
  // never reach this point
  rha_error("resolve_assign_prule called without assign symbol.\n");
  return 0;
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


/*
 *  if
 */

BUILTIN(b_op_if)  // will be if_fn in rhabarber
{
  int nin = tuple_length(in);
  if (nin == 3 || nin == 4) {
    object_t cond = eval(env, tuple_get(in, 1));
    assert(cond);
    if (!HAS_TYPE(bool, cond))
      rha_error("The condition of 'if' must be of type 'bool'.\n");
    if(bool_get(cond))
      return eval(env, tuple_get(in, 2));
    else if(nin == 4)
      return eval(env, tuple_get(in, 3));
    else
      return none_obj;
  }
  return 0;
}


/*
 *  do
 */


BUILTIN(b_op_return)
{
  int nin = tuple_length(in);
  if (nin==2) {
    frame_jump(FUNCTION_FRAME, tuple_get(in, 1));
    // never reaches this point
  }
  else if (nin==1) {
    frame_jump(FUNCTION_FRAME, none_obj);
    // never reaches this point
  }
  return 0;
}


BUILTIN(b_op_deliver)
{
  int nin = tuple_length(in);
  if (nin==2) {
    frame_jump(BLOCK_FRAME, tuple_get(in, 1));
    // never reaches this point
  }
  else if (nin==1) {
    frame_jump(BLOCK_FRAME, none_obj);
    // never reaches this point
  }
  return 0;
}


BUILTIN(b_op_break)
{
  int nin = tuple_length(in);
  if (nin==2) {
    frame_jump(LOOP_FRAME, tuple_get(in, 1));
    // never reaches this point
  }
  else if (nin==1) {
    frame_jump(LOOP_FRAME, none_obj);
    // never reaches this point
  }
  return 0;
}


BUILTIN(b_op_throw)
{
  int nin = tuple_length(in);
  if (nin==2) {
    throw(eval(env, tuple_get(in, 1)));
    // never reaches this point
  }
  return 0;
}


BUILTIN(b_eval)
{
  int nin = tuple_length(in);
  if (nin==2) {
    return eval(env, tuple_get(in, 1));
  }
  else if (nin==3) {
    return eval(tuple_get(in, 1), tuple_get(in, 2));
  }
  return 0;
}


BUILTIN(b_delete)
{
  int nin = tuple_length(in);
  if (nin==2) {
    object_t t1 = tuple_get(in, 1);
    if (!HAS_TYPE(symbol, t1))
      rha_error("Calling 'delete' with one arg requires arg to be a symbol (try backslash, e.g. delete(\\x)).\n");
    object_delete(env, t1);
    return none_new();
  }
  else if (nin==3) {
    object_t t2 = tuple_get(in, 2);
    if (!HAS_TYPE(symbol, t2))
      rha_error("Calling 'delete' with two args requires the 2nd arg to be a symbol (try backslash, e.g. delete(\\x)).\n");
    object_delete(tuple_get(in, 1), t2);
    return none_new();
  }
  return 0;
}


BUILTIN(b_do)
{
  int nin = tuple_length(in);
  if (nin>0) {  
    // at least one entries, e.g. (do) 
    // returns void or a return value ('return 17')
    // doesn't open a new scope
    object_t res = none_obj;

    begin_frame(BLOCK_FRAME) {
      // evaluate all
      for (int i = 1; i<nin; i++) {
        eval(env, tuple_get(in, i));
      }
    }
    end_frame(res);
    // return the result, which might be 'none'
    return res;
  }
  return 0;
}


BUILTIN(b_exit)
{
  int nin = tuple_length(in);
  if (nin == 1) { 
    saybye();
    exit(EXIT_SUCCESS); 
  }
  else if (nin ==2) {
    object_t t1 = tuple_get(in, 1);
    if (HAS_TYPE(int, t1)) {
      saybye();
      exit(int_get(t1));
    }
  }
  rha_error("Only exit() or exit(:int) possible.\n");
  return 0;
}


/*
 *  tuple
 */
BUILTIN(b_tuplefy)
{
  // we only have to chop off the first element
  int nin = tuple_length(in);
  int i = 0;
  tuple_tr t = tuple_new(nin-1);
  for (i = 1; i < nin; i++)
    tuple_set(t, i-1, tuple_get(in, i));
  return t;
}


/*
 *  while
 */

BUILTIN(b_op_while)
{
  int nin = tuple_length(in);
  if (nin == 3) {
    object_t res = none_obj; // must be initialized for "cond==false"
    object_t body = tuple_get(in, 2);
    begin_frame(LOOP_FRAME)
      while (1) {
	object_t cond = eval(env, tuple_get(in, 1));
	if (!cond || !HAS_TYPE(bool, cond))
	  return 0;
	else if (!bool_get(cond)) 
	  break;
	else
	  res = eval(env, body);
      }
    end_frame(res);
    return res;
  }
  return 0;
}


BUILTIN(b_op_try)
{
  int nin = tuple_length(in);
  if (nin == 4) {
    object_t tryblock = tuple_get(in, 1);
    object_t catchvar = tuple_get(in, 2);
    object_t catchblock = tuple_get(in, 3);

    // check catchvar
    if (!HAS_TYPE(symbol, catchvar)) {
      catchvar = eval(env, catchvar);
      if (!HAS_TYPE(symbol, catchvar)) {
	// note that this exception will be thrown to the latest
	// try/catch block, not to the new one.
	rha_error("The argument passed to 'catch' must be a symbol or must evaluate to a symbol.\n");
      }
    }

    // the result will be the return value of 
    //       the tryblock if no exception is thrown
    //       the catchblock if an exception is thrown
    object_t res = none_new();
    object_t excp;

    // let's try it
    try 
      res = eval(env, tryblock);
    catch(excp) {
      object_assign(env, catchvar, excp);
      res = eval(env, catchblock);
    }

    // return the result ignoring which block has created it
    return res;
  }
  return 0;
}


BUILTIN(b_op_for_iterator)
{
  int nin = tuple_length(in);
  if (nin == 4) {
    // get the element symbol and the container
    object_t obj_sym = tuple_get(in, 1);
    if (!HAS_TYPE(symbol, obj_sym)) {
      obj_sym = eval(env, obj_sym);
      if (!HAS_TYPE(symbol, obj_sym)) 
	rha_error("op_for_iterator: first arg must be symbol (or eval to a symbol).\n");
    }
    // at this point we could force the iterator variable to be local
    // let's force it to be a local var
    //    string_t str = symbol_name(obj_sym);
    //    if (str[0] != '@' && str[0] != '$') 
    //      obj_sym = symbol_new(string_plus_string("@", str));
    object_t container = eval(env, tuple_get(in, 2));
    object_t iter = object_callslot(container, "iter", 0);
    object_t body = tuple_get(in, 3);
    object_t inner = object_clone(env);
    object_t res = none_obj;
    
    // run the loop
    begin_frame(LOOP_FRAME)
      while(!bool_get(object_callslot(iter, "done", 0))) {
	assigntosymbol(inner, obj_sym, object_callslot(iter, "current", 0));
	res = eval(inner, body);
	object_callslot(iter, "next", 0);
      }
    end_frame(res);
    return res;
  }
  return 0;
}


BUILTIN(b_op_for_cstyle)
{
  int nin = tuple_length(in);
  if (nin == 5) {
    // get the init, check and update
    object_t init = tuple_get(in, 1);
    object_t check = tuple_get(in, 2);
    object_t update = tuple_get(in, 3);
    object_t body = tuple_get(in, 4);
    object_t inner = object_clone(env);
    object_t res = none_obj;

    // run the loop
    begin_frame(LOOP_FRAME) {
      eval(inner, init);
      while(bool_get(eval(inner, check))) {
	res = eval(inner, body);
	eval(inner, update);
      }
    }
    end_frame(res);
    return res;
  }
  return 0;
}

/*
 *  clone
 */

BUILTIN(b_clone)
{
  int nin = tuple_length(in);
  if (nin==2) {
    object_t parent = eval(env, tuple_get(in, nin-1));
    if (parent) return object_clone(parent);
  }
  return 0;
}

/*
 *
 * Loading a rha-file
 *
 */

BUILTIN(b_load)
{
  object_t obj = 0;
  for(int i = 1; i < tuple_length(in); i++) {
    object_t fn = tuple_get(in, i);
    if(HAS_TYPE(string, fn)) {
      list_tr code = rhaparsefile(string_get(fn));
      if(code) obj = resolve_eval_list(env, code);
    } 
  }
  return 0; // used to be return obj;
}


BUILTIN(b_op_import)
{
  int nin = tuple_length(in);
  if (nin == 2) {
    object_t fn = tuple_get(in, 1);
    if(HAS_TYPE(symbol, fn)) {
      // already loaded?
      object_t modules = object_lookup(root, symbol_new("modules"));
      assert(modules); // should have been constructed on init
      object_t module = object_lookup(modules, fn);
      if(!module) {
	module = object_clone(root);
	string_t fnrha = string_plus_string(symbol_name(fn), ".rha");
	list_tr code = rhaparsefile(fnrha);
	if(code) {
	  resolve_eval_list(module, code);
	  list_append(path, module);
	  object_assign(modules, fn, module);
	  return none_obj;
	}
	else
	  rha_error("Couldn't load module \"%o\"\n", _O fn);
      }
    } 
  }
  return 0; // used to be return obj;
}


BUILTIN(b_print)
{
  for(int i = 1; i < tuple_length(in); i++)
    print("%o", tuple_get(in, i));
  print("\n");
  return 0;
}


BUILTIN(b_slots)
{
  if (tuple_length(in) == 2) {
    object_t o = tuple_get(in, 1);
    if (!o) 
      fprintf(stdout, "Object is nil.\n");
    else
      return object_slots(o);
  }
  return 0;
}


BUILTIN(b_allslots)
{
  if (tuple_length(in) == 2) {
    object_t o = tuple_get(in, 1);
    if (!o) 
      fprintf(stdout, "Object is nil.\n");
    else
      return object_allslots(o);
  }
  return 0;
}

// some special function for increment and decrement
// note that they are passed the symbol

BUILTIN(b_preplusplus)
{
  if (tuple_length(in) == 2) {
    object_t s = tuple_get(in, 1);
    if (!HAS_TYPE(symbol, s)) {
      s = eval(env, s);
      if (!HAS_TYPE(symbol, s)) {
	rha_error("x must be a symbol (or must evaluate to a symbol) in ++x.\n");
      }
    }
    object_t obj = rha_lookup(env, s);
    if (!obj) rha_error("x must already exist to call ++x.\n");
    return rha_replace(env, s, object_callslot(obj, "inc", 0));
  }
  return 0;
}

BUILTIN(b_preminusminus)
{
  if (tuple_length(in) == 2) {
    object_t s = tuple_get(in, 1);
    if (!HAS_TYPE(symbol, s)) {
      s = eval(env, s);
      if (!HAS_TYPE(symbol, s)) {
	rha_error("x must be a symbol (or must evaluate to a symbol) in ++x.\n");
      }
    }
    object_t obj = rha_lookup(env, s);
    if (!obj) rha_error("x must already exist to call ++x.\n");
    return rha_replace(env, s, object_callslot(obj, "dec", 0));
  }
  return 0;
}

BUILTIN(b_postplusplus)
{
  if (tuple_length(in) == 2) {
    object_t s = tuple_get(in, 1);
    if (!HAS_TYPE(symbol, s)) {
      s = eval(env, s);
      if (!HAS_TYPE(symbol, s)) {
	rha_error("x must be a symbol (or must evaluate to a symbol) in ++x.\n");
      }
    }
    object_t obj = rha_lookup(env, s);
    if (!obj) rha_error("x must already exist to call ++x.\n");
    object_t y = object_callslot(obj, "copy", 0);
    rha_replace(env, s, object_callslot(obj, "inc", 0));
    return y;
  }
  return 0;
}

BUILTIN(b_postminusminus)
{
  if (tuple_length(in) == 2) {
    object_t s = tuple_get(in, 1);
    if (!HAS_TYPE(symbol, s)) {
      s = eval(env, s);
      if (!HAS_TYPE(symbol, s)) {
	rha_error("x must be a symbol (or must evaluate to a symbol) in ++x.\n");
      }
    }
    object_t obj = rha_lookup(env, s);
    if (!obj) rha_error("x must already exist to call ++x.\n");
    object_t y = object_callslot(obj, "copy", 0);
    rha_replace(env, s, object_callslot(obj, "dec", 0));
    return y;
  }
  return 0;
}



BUILTIN(b_addr)
{
  if (tuple_length(in) == 2) {
    return int_new((int)tuple_get(in, 1));
  }
  else
    rha_error("addr(x) expects only one argument.");
  return 0;
}


BUILTIN(b_error)
{
  string_t s = gc_strdup("");
  for(int i = 1; i < tuple_length(in); i++)
    s = sprint("%s%o", s, tuple_get(in, i));
  rha_error(sprint("%s\n", s));
  return 0;  
}


BUILTIN(b_id)
{
  if (tuple_length(in) == 2) {
    return tuple_get(in, 1);
  }
  else
    return tuple_shiftfirst(in);
}

BUILTIN(b_snatch)
{
  if (tuple_length(in) == 3) {
    object_snatch(tuple_get(in, 1), tuple_get(in, 2));
    return tuple_get(in , 1);
  }
  return 0;
}


/************************************************************
 *
 * For time measurement purposes
 *
 *************************************************************/

#include <time.h>
#include <sys/time.h>

int tic_saved_sec;
double tic_saved_time;

BUILTIN(b_tic)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
    
  tic_saved_sec = tv.tv_sec;
  tic_saved_time = tv.tv_usec * 1e-6;

  return none_obj;
}

BUILTIN(b_toc)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  tv.tv_sec -= tic_saved_sec;
  double now = tv.tv_sec + tv.tv_usec*1e-6;

  return real_new(now - tic_saved_time);
}
