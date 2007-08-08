#include "parse.h"


object_t parse(string_t s);

object_t add_keyword(string_t s, int_t p) {
  symbol_t key = symbol_new(s);
  object_t o = new();
  assign(o, symbol_new("token"), key);
  assign(o, symbol_new("precedence"), p);
  return o;
}

#define MACROFIX 0
#define POSTFIX  1
#define PREFIX   2
#define INFIX    3
#define FREEFIX  4

#define MACROPREC   1.0
#define POSTPREC    2.0
// INFIX should be between 3.0 and 4.0 for numerical infix operators
#define PREPREC     3.0
#define FREEPREC    5.0
// INFIX should be larger than 5.0 for assignments and comma, semicolon

#define MAKE_PRULE(ttt)\
  object_t o = new();\
  assign(o, symbol_new("token"), symbol_new(s));\
  assign(o, symbol_new("precedence"), ttt ## PREC);\
  assign(o, symbol_new("style"), ttt ## FIX);

object_t add_macro_prule(string_t s, object_t f) {
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

