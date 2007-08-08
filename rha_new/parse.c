#include "parse.h"
#include "rha_parser.h" // the link to BISON

object_t parse(string_t s){

  // (0) run bison code
  list_t wslist = rhaparsestring(s);
  
  // (1) resolve function calls
  // go from left to right
  // if there is a keyword, ignore depending on its arity
  // otherwise built function call
  
  
  // (2) resolve prule from inside out
  // go from right to left
  // use a stack and some magic

}

object_t add_keyword(string_t s, int_t a) {
  object_t o = new();
  assign(o, symbol_new("token"), symbol_new(s));
  assign(o, symbol_new("arity"), a);
  return o;
}

#define MACROFIX 0
#define POSTFIX  1
#define PREFIX   2
#define INFIX    3
#define FREEFIX  4

#define MACROPREC   1.0
#define POSTPREC    2.0
#define PREPREC     3.0
// INFIX should be between 3.0 and 5.0 for numerical infix operators
#define FREEPREC    5.0
// INFIX should be larger than 5.0 for assignments
// INFIX for comma, semicolon even larger

#define MAKE_PRULE(ttt)\
  object_t o = new();\
  assign(o, symbol_new("token"), symbol_new(s));\
  assign(o, symbol_new("code"), f);\
  assign(o, symbol_new("precedence"), ttt ## PREC);\
  assign(o, symbol_new("style"), ttt ## FIX);

object_t add_macrofix_prule(string_t s, object_t f) {
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

