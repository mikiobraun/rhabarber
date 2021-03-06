/*
 * core_symbols.h - Symbol table declarations, definitions and assignments
 *
 * This file has been generated automatically with makesymbols.pl
 * from the file core_symbols.
 *
 * DO NOT EDIT THIS FILE! Edit core_symbols instead.
 *
 */

/*
 * Usage:
 * 
 * you can include this file three times, by defining
 *
 * CORE_SYMBOLS_DECLARE - extern declarations of the symbols
 * CORE_SYMBOLS_DEFINE  - define the symbols
 * CORE_SYMBOLS_INIT    - initialize the symbols
 * CORE_SYMBOLS_ADD     - add the symbols to the symbol table of root
 *
 * This should be done like this:
 *
 * #define CORE_SYMBOLS_DECLARE
 * #include "core_symbols.h"
 * #undef CORE_SYMBOLS_DECLARE
 *
 * etc.
 */

#ifdef CORE_SYMBOLS_DECLARE
#line 1 "core_symbols"
// e.g.  'if' is a prule (parse rule); 'op_if' is a function/macro
extern symbol_tr dot_sym;
extern symbol_tr op_dot_sym;
extern symbol_tr nobinddot_sym;
extern symbol_tr op_nobinddot_sym;
extern symbol_tr quest_sym;
extern symbol_tr op_quest_sym;
extern symbol_tr fn_sym;
extern symbol_tr op_fn_sym;
extern symbol_tr macro_sym;
extern symbol_tr op_macro_sym;
extern symbol_tr prule_sym;
extern symbol_tr op_prule_sym;
extern symbol_tr if_sym;
extern symbol_tr op_if_sym;
extern symbol_tr while_sym;
extern symbol_tr op_while_sym;
extern symbol_tr for_sym;
extern symbol_tr op_for_iterator_sym;
extern symbol_tr op_for_cstyle_sym;
extern symbol_tr return_sym;
extern symbol_tr op_return_sym;
extern symbol_tr deliver_sym;
extern symbol_tr op_deliver_sym;
extern symbol_tr break_sym;
extern symbol_tr op_break_sym;
extern symbol_tr import_sym;
extern symbol_tr op_import_sym;
extern symbol_tr try_sym;
extern symbol_tr op_try_sym;
extern symbol_tr throw_sym;
extern symbol_tr op_throw_sym;

// auxiliary symbols (used inside parse rules)
extern symbol_tr in_sym;
extern symbol_tr else_sym;
extern symbol_tr catch_sym;

// symbols for typing
extern symbol_tr type_sym;
extern symbol_tr typename_sym;

// other builtin functions
extern symbol_tr snatch_sym;
extern symbol_tr do_sym;
extern symbol_tr exit_sym;
extern symbol_tr eval_sym;
extern symbol_tr delete_sym;
extern symbol_tr tuplefy_sym;
extern symbol_tr true_sym;
extern symbol_tr false_sym;
extern symbol_tr group_sym;
extern symbol_tr id_sym;

// symbols for literal-separator, 
// it would be more consequent to have: comma   ,
// but this would hard to read when tuple are printed
extern symbol_tr comma_sym;
extern symbol_tr semic_sym;
extern symbol_tr colon_sym;

// special objects
extern symbol_tr from_sym;
extern symbol_tr parent_sym;
extern symbol_tr this_sym;
extern symbol_tr that_sym;
extern symbol_tr root_sym;
extern symbol_tr env_sym;

extern symbol_tr rha_location_sym;
extern symbol_tr call_sym;
extern symbol_tr calldef_sym;

// operators
extern symbol_tr quote_sym;
extern symbol_tr op_quote_sym;
extern symbol_tr assign_sym;
extern symbol_tr op_assign_sym;
extern symbol_tr plusassign_sym;
extern symbol_tr minusassign_sym;
extern symbol_tr timesassign_sym;
extern symbol_tr divideassign_sym;
extern symbol_tr plus_sym;
extern symbol_tr op_plus_sym;
extern symbol_tr minus_sym;
extern symbol_tr op_minus_sym;
extern symbol_tr times_sym;
extern symbol_tr op_times_sym;
extern symbol_tr divide_sym;
extern symbol_tr op_divide_sym;
extern symbol_tr equal_sym;
extern symbol_tr op_equal_sym;
extern symbol_tr notequal_sym;
extern symbol_tr op_notequal_sym;
extern symbol_tr greater_sym;
extern symbol_tr op_greater_sym;
extern symbol_tr greaterequal_sym;
extern symbol_tr op_greaterequal_sym;
extern symbol_tr less_sym;
extern symbol_tr op_less_sym;
extern symbol_tr lessequal_sym;
extern symbol_tr op_lessequal_sym;
extern symbol_tr and_sym;
extern symbol_tr op_and_sym;
extern symbol_tr or_sym;
extern symbol_tr op_or_sym;
extern symbol_tr not_sym;
extern symbol_tr op_not_sym;
extern symbol_tr literal_sym;
extern symbol_tr copy_sym;

// note that ++ is special, it can be manipulated via int.inc
extern symbol_tr inc_sym;
extern symbol_tr plusplus_sym;
extern symbol_tr preplusplus_sym;
extern symbol_tr postplusplus_sym;
extern symbol_tr dec_sym;
extern symbol_tr minusminus_sym;
extern symbol_tr preminusminus_sym;
extern symbol_tr postminusminus_sym;
#endif

#ifdef CORE_SYMBOLS_DEFINE
#line 1 "core_symbols"
// e.g.  'if' is a prule (parse rule); 'op_if' is a function/macro
symbol_tr dot_sym = 0;
symbol_tr op_dot_sym = 0;
symbol_tr nobinddot_sym = 0;
symbol_tr op_nobinddot_sym = 0;
symbol_tr quest_sym = 0;
symbol_tr op_quest_sym = 0;
symbol_tr fn_sym = 0;
symbol_tr op_fn_sym = 0;
symbol_tr macro_sym = 0;
symbol_tr op_macro_sym = 0;
symbol_tr prule_sym = 0;
symbol_tr op_prule_sym = 0;
symbol_tr if_sym = 0;
symbol_tr op_if_sym = 0;
symbol_tr while_sym = 0;
symbol_tr op_while_sym = 0;
symbol_tr for_sym = 0;
symbol_tr op_for_iterator_sym = 0;
symbol_tr op_for_cstyle_sym = 0;
symbol_tr return_sym = 0;
symbol_tr op_return_sym = 0;
symbol_tr deliver_sym = 0;
symbol_tr op_deliver_sym = 0;
symbol_tr break_sym = 0;
symbol_tr op_break_sym = 0;
symbol_tr import_sym = 0;
symbol_tr op_import_sym = 0;
symbol_tr try_sym = 0;
symbol_tr op_try_sym = 0;
symbol_tr throw_sym = 0;
symbol_tr op_throw_sym = 0;

// auxiliary symbols (used inside parse rules)
symbol_tr in_sym = 0;
symbol_tr else_sym = 0;
symbol_tr catch_sym = 0;

// symbols for typing
symbol_tr type_sym = 0;
symbol_tr typename_sym = 0;

// other builtin functions
symbol_tr snatch_sym = 0;
symbol_tr do_sym = 0;
symbol_tr exit_sym = 0;
symbol_tr eval_sym = 0;
symbol_tr delete_sym = 0;
symbol_tr tuplefy_sym = 0;
symbol_tr true_sym = 0;
symbol_tr false_sym = 0;
symbol_tr group_sym = 0;
symbol_tr id_sym = 0;

// symbols for literal-separator, 
// it would be more consequent to have: comma   ,
// but this would hard to read when tuple are printed
symbol_tr comma_sym = 0;
symbol_tr semic_sym = 0;
symbol_tr colon_sym = 0;

// special objects
symbol_tr from_sym = 0;
symbol_tr parent_sym = 0;
symbol_tr this_sym = 0;
symbol_tr that_sym = 0;
symbol_tr root_sym = 0;
symbol_tr env_sym = 0;

symbol_tr rha_location_sym = 0;
symbol_tr call_sym = 0;
symbol_tr calldef_sym = 0;

// operators
symbol_tr quote_sym = 0;
symbol_tr op_quote_sym = 0;
symbol_tr assign_sym = 0;
symbol_tr op_assign_sym = 0;
symbol_tr plusassign_sym = 0;
symbol_tr minusassign_sym = 0;
symbol_tr timesassign_sym = 0;
symbol_tr divideassign_sym = 0;
symbol_tr plus_sym = 0;
symbol_tr op_plus_sym = 0;
symbol_tr minus_sym = 0;
symbol_tr op_minus_sym = 0;
symbol_tr times_sym = 0;
symbol_tr op_times_sym = 0;
symbol_tr divide_sym = 0;
symbol_tr op_divide_sym = 0;
symbol_tr equal_sym = 0;
symbol_tr op_equal_sym = 0;
symbol_tr notequal_sym = 0;
symbol_tr op_notequal_sym = 0;
symbol_tr greater_sym = 0;
symbol_tr op_greater_sym = 0;
symbol_tr greaterequal_sym = 0;
symbol_tr op_greaterequal_sym = 0;
symbol_tr less_sym = 0;
symbol_tr op_less_sym = 0;
symbol_tr lessequal_sym = 0;
symbol_tr op_lessequal_sym = 0;
symbol_tr and_sym = 0;
symbol_tr op_and_sym = 0;
symbol_tr or_sym = 0;
symbol_tr op_or_sym = 0;
symbol_tr not_sym = 0;
symbol_tr op_not_sym = 0;
symbol_tr literal_sym = 0;
symbol_tr copy_sym = 0;

// note that ++ is special, it can be manipulated via int.inc
symbol_tr inc_sym = 0;
symbol_tr plusplus_sym = 0;
symbol_tr preplusplus_sym = 0;
symbol_tr postplusplus_sym = 0;
symbol_tr dec_sym = 0;
symbol_tr minusminus_sym = 0;
symbol_tr preminusminus_sym = 0;
symbol_tr postminusminus_sym = 0;
#endif

#ifdef CORE_SYMBOLS_INIT
#line 1 "core_symbols"
  // e.g.  'if' is a prule (parse rule); 'op_if' is a function/macro
  dot_sym = symbol_new(".");
  op_dot_sym = symbol_new("op.");
  nobinddot_sym = symbol_new(".!");
  op_nobinddot_sym = symbol_new("op.!");
  quest_sym = symbol_new("?");
  op_quest_sym = symbol_new("op?");
  fn_sym = symbol_new("fn");
  op_fn_sym = symbol_new("op_fn");
  macro_sym = symbol_new("macro");
  op_macro_sym = symbol_new("op_macro");
  prule_sym = symbol_new("prule");
  op_prule_sym = symbol_new("op_prule");
  if_sym = symbol_new("if");
  op_if_sym = symbol_new("op_if");
  while_sym = symbol_new("while");
  op_while_sym = symbol_new("op_while");
  for_sym = symbol_new("for");
  op_for_iterator_sym = symbol_new("op_for_iterator");
  op_for_cstyle_sym = symbol_new("op_for_cstyle");
  return_sym = symbol_new("return");
  op_return_sym = symbol_new("op_return");
  deliver_sym = symbol_new("deliver");
  op_deliver_sym = symbol_new("op_deliver");
  break_sym = symbol_new("break");
  op_break_sym = symbol_new("op_break");
  import_sym = symbol_new("import");
  op_import_sym = symbol_new("op_import");
  try_sym = symbol_new("try");
  op_try_sym = symbol_new("op_try");
  throw_sym = symbol_new("throw");
  op_throw_sym = symbol_new("op_throw");

  // auxiliary symbols (used inside parse rules)
  in_sym = symbol_new("in");
  else_sym = symbol_new("else");
  catch_sym = symbol_new("catch");

  // symbols for typing
  type_sym = symbol_new("type");
  typename_sym = symbol_new("typename");

  // other builtin functions
  snatch_sym = symbol_new("=?");
  do_sym = symbol_new("do");
  exit_sym = symbol_new("exit");
  eval_sym = symbol_new("eval");
  delete_sym = symbol_new("delete");
  tuplefy_sym = symbol_new("tuplefy");
  true_sym = symbol_new("true");
  false_sym = symbol_new("false");
  group_sym = symbol_new("group");
  id_sym = symbol_new("id");

  // symbols for literal-separator, 
  // it would be more consequent to have: comma   ,
  // but this would hard to read when tuple are printed
  comma_sym = symbol_new("comma");
  semic_sym = symbol_new("semic");
  colon_sym = symbol_new("colon");

  // special objects
  from_sym = symbol_new("from");
  parent_sym = symbol_new("parent");
  this_sym = symbol_new("this");
  that_sym = symbol_new("that");
  root_sym = symbol_new("root");
  env_sym = symbol_new("env");

  rha_location_sym = symbol_new("rha_location");
  call_sym = symbol_new("rha_call");
  calldef_sym = symbol_new("rha_calldef");

  // operators
  quote_sym = symbol_new("\\");
  op_quote_sym = symbol_new("op\\");
  assign_sym = symbol_new("=");
  op_assign_sym = symbol_new("op=");
  plusassign_sym = symbol_new("+=");
  minusassign_sym = symbol_new("-=");
  timesassign_sym = symbol_new("*=");
  divideassign_sym = symbol_new("/=");
  plus_sym = symbol_new("+");
  op_plus_sym = symbol_new("op+");
  minus_sym = symbol_new("-");
  op_minus_sym = symbol_new("op-");
  times_sym = symbol_new("*");
  op_times_sym = symbol_new("op*");
  divide_sym = symbol_new("/");
  op_divide_sym = symbol_new("op/");
  equal_sym = symbol_new("==");
  op_equal_sym = symbol_new("op==");
  notequal_sym = symbol_new("!=");
  op_notequal_sym = symbol_new("op!=");
  greater_sym = symbol_new(">");
  op_greater_sym = symbol_new("op>");
  greaterequal_sym = symbol_new(">=");
  op_greaterequal_sym = symbol_new("op>=");
  less_sym = symbol_new("<");
  op_less_sym = symbol_new("op<");
  lessequal_sym = symbol_new("<=");
  op_lessequal_sym = symbol_new("op<=");
  and_sym = symbol_new("&&");
  op_and_sym = symbol_new("op&&");
  or_sym = symbol_new("||");
  op_or_sym = symbol_new("op||");
  not_sym = symbol_new("!");
  op_not_sym = symbol_new("op!");
  literal_sym = symbol_new("literal");
  copy_sym = symbol_new("copy");

  // note that ++ is special, it can be manipulated via int.inc
  inc_sym = symbol_new("inc");
  plusplus_sym = symbol_new("++");
  preplusplus_sym = symbol_new("preplusplus");
  postplusplus_sym = symbol_new("postplusplus");
  dec_sym = symbol_new("dec");
  minusminus_sym = symbol_new("--");
  preminusminus_sym = symbol_new("preminusminus");
  postminusminus_sym = symbol_new("postminusminus");
#endif

#ifdef CORE_SYMBOLS_ADD
#line 1 "core_symbols"
  // e.g.  'if' is a prule (parse rule); 'op_if' is a function/macro
  object_assign(root, symbol_new("dot_sym"), dot_sym);
  object_assign(root, symbol_new("op_dot_sym"), op_dot_sym);
  object_assign(root, symbol_new("nobinddot_sym"), nobinddot_sym);
  object_assign(root, symbol_new("op_nobinddot_sym"), op_nobinddot_sym);
  object_assign(root, symbol_new("quest_sym"), quest_sym);
  object_assign(root, symbol_new("op_quest_sym"), op_quest_sym);
  object_assign(root, symbol_new("fn_sym"), fn_sym);
  object_assign(root, symbol_new("op_fn_sym"), op_fn_sym);
  object_assign(root, symbol_new("macro_sym"), macro_sym);
  object_assign(root, symbol_new("op_macro_sym"), op_macro_sym);
  object_assign(root, symbol_new("prule_sym"), prule_sym);
  object_assign(root, symbol_new("op_prule_sym"), op_prule_sym);
  object_assign(root, symbol_new("if_sym"), if_sym);
  object_assign(root, symbol_new("op_if_sym"), op_if_sym);
  object_assign(root, symbol_new("while_sym"), while_sym);
  object_assign(root, symbol_new("op_while_sym"), op_while_sym);
  object_assign(root, symbol_new("for_sym"), for_sym);
  object_assign(root, symbol_new("op_for_iterator_sym"), op_for_iterator_sym);
  object_assign(root, symbol_new("op_for_cstyle_sym"), op_for_cstyle_sym);
  object_assign(root, symbol_new("return_sym"), return_sym);
  object_assign(root, symbol_new("op_return_sym"), op_return_sym);
  object_assign(root, symbol_new("deliver_sym"), deliver_sym);
  object_assign(root, symbol_new("op_deliver_sym"), op_deliver_sym);
  object_assign(root, symbol_new("break_sym"), break_sym);
  object_assign(root, symbol_new("op_break_sym"), op_break_sym);
  object_assign(root, symbol_new("import_sym"), import_sym);
  object_assign(root, symbol_new("op_import_sym"), op_import_sym);
  object_assign(root, symbol_new("try_sym"), try_sym);
  object_assign(root, symbol_new("op_try_sym"), op_try_sym);
  object_assign(root, symbol_new("throw_sym"), throw_sym);
  object_assign(root, symbol_new("op_throw_sym"), op_throw_sym);

  // auxiliary symbols (used inside parse rules)
  object_assign(root, symbol_new("in_sym"), in_sym);
  object_assign(root, symbol_new("else_sym"), else_sym);
  object_assign(root, symbol_new("catch_sym"), catch_sym);

  // symbols for typing
  object_assign(root, symbol_new("type_sym"), type_sym);
  object_assign(root, symbol_new("typename_sym"), typename_sym);

  // other builtin functions
  object_assign(root, symbol_new("snatch_sym"), snatch_sym);
  object_assign(root, symbol_new("do_sym"), do_sym);
  object_assign(root, symbol_new("exit_sym"), exit_sym);
  object_assign(root, symbol_new("eval_sym"), eval_sym);
  object_assign(root, symbol_new("delete_sym"), delete_sym);
  object_assign(root, symbol_new("tuplefy_sym"), tuplefy_sym);
  object_assign(root, symbol_new("true_sym"), true_sym);
  object_assign(root, symbol_new("false_sym"), false_sym);
  object_assign(root, symbol_new("group_sym"), group_sym);
  object_assign(root, symbol_new("id_sym"), id_sym);

  // symbols for literal-separator, 
  // it would be more consequent to have: comma   ,
  // but this would hard to read when tuple are printed
  object_assign(root, symbol_new("comma_sym"), comma_sym);
  object_assign(root, symbol_new("semic_sym"), semic_sym);
  object_assign(root, symbol_new("colon_sym"), colon_sym);

  // special objects
  object_assign(root, symbol_new("from_sym"), from_sym);
  object_assign(root, symbol_new("parent_sym"), parent_sym);
  object_assign(root, symbol_new("this_sym"), this_sym);
  object_assign(root, symbol_new("that_sym"), that_sym);
  object_assign(root, symbol_new("root_sym"), root_sym);
  object_assign(root, symbol_new("env_sym"), env_sym);

  object_assign(root, symbol_new("rha_location_sym"), rha_location_sym);
  object_assign(root, symbol_new("call_sym"), call_sym);
  object_assign(root, symbol_new("calldef_sym"), calldef_sym);

  // operators
  object_assign(root, symbol_new("quote_sym"), quote_sym);
  object_assign(root, symbol_new("op_quote_sym"), op_quote_sym);
  object_assign(root, symbol_new("assign_sym"), assign_sym);
  object_assign(root, symbol_new("op_assign_sym"), op_assign_sym);
  object_assign(root, symbol_new("plusassign_sym"), plusassign_sym);
  object_assign(root, symbol_new("minusassign_sym"), minusassign_sym);
  object_assign(root, symbol_new("timesassign_sym"), timesassign_sym);
  object_assign(root, symbol_new("divideassign_sym"), divideassign_sym);
  object_assign(root, symbol_new("plus_sym"), plus_sym);
  object_assign(root, symbol_new("op_plus_sym"), op_plus_sym);
  object_assign(root, symbol_new("minus_sym"), minus_sym);
  object_assign(root, symbol_new("op_minus_sym"), op_minus_sym);
  object_assign(root, symbol_new("times_sym"), times_sym);
  object_assign(root, symbol_new("op_times_sym"), op_times_sym);
  object_assign(root, symbol_new("divide_sym"), divide_sym);
  object_assign(root, symbol_new("op_divide_sym"), op_divide_sym);
  object_assign(root, symbol_new("equal_sym"), equal_sym);
  object_assign(root, symbol_new("op_equal_sym"), op_equal_sym);
  object_assign(root, symbol_new("notequal_sym"), notequal_sym);
  object_assign(root, symbol_new("op_notequal_sym"), op_notequal_sym);
  object_assign(root, symbol_new("greater_sym"), greater_sym);
  object_assign(root, symbol_new("op_greater_sym"), op_greater_sym);
  object_assign(root, symbol_new("greaterequal_sym"), greaterequal_sym);
  object_assign(root, symbol_new("op_greaterequal_sym"), op_greaterequal_sym);
  object_assign(root, symbol_new("less_sym"), less_sym);
  object_assign(root, symbol_new("op_less_sym"), op_less_sym);
  object_assign(root, symbol_new("lessequal_sym"), lessequal_sym);
  object_assign(root, symbol_new("op_lessequal_sym"), op_lessequal_sym);
  object_assign(root, symbol_new("and_sym"), and_sym);
  object_assign(root, symbol_new("op_and_sym"), op_and_sym);
  object_assign(root, symbol_new("or_sym"), or_sym);
  object_assign(root, symbol_new("op_or_sym"), op_or_sym);
  object_assign(root, symbol_new("not_sym"), not_sym);
  object_assign(root, symbol_new("op_not_sym"), op_not_sym);
  object_assign(root, symbol_new("literal_sym"), literal_sym);
  object_assign(root, symbol_new("copy_sym"), copy_sym);

  // note that ++ is special, it can be manipulated via int.inc
  object_assign(root, symbol_new("inc_sym"), inc_sym);
  object_assign(root, symbol_new("plusplus_sym"), plusplus_sym);
  object_assign(root, symbol_new("preplusplus_sym"), preplusplus_sym);
  object_assign(root, symbol_new("postplusplus_sym"), postplusplus_sym);
  object_assign(root, symbol_new("dec_sym"), dec_sym);
  object_assign(root, symbol_new("minusminus_sym"), minusminus_sym);
  object_assign(root, symbol_new("preminusminus_sym"), preminusminus_sym);
  object_assign(root, symbol_new("postminusminus_sym"), postminusminus_sym);
#endif

