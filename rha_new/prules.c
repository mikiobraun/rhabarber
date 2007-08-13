#include "prules.h"

#include "core.h"

// prules take a white-space list and return a function call!
// prules must not return a macro call!


#define IGNORE
#ifndef IGNORE
bool_t iscallof(symbol_t s, object_t t)
{
  symbol_t t0 = 0;
  return (ptype(t) == TUPLE_T)
    && (tuple_len(t) > 0)
    && (ptype(t0=tuple_get(t, 0)) == SYMBOL_T) 
    && (s == t0);
}
#endif

bool_t debug = true;

tuple_t dot(list_t parsetree) {
  return resolve_infix_prule(parsetree, dot_sym, lookup_sym, LEFT_BIND);
}

tuple_t plus(list_t parsetree) {
  return resolve_infix_prule(parsetree, plus_sym, int_plus_sym, LEFT_BIND);
}





object_t prules_init(object_t root, object_t module) {

  // add priority slots for all prules
  // and add those prules also as symbols
  object_t f = 0;
  f = lookup(module, dot_sym);
  assign(f, priority_sym, WRAP_REAL(0.5));
  assign(module, symbol_new("."), f);
  f = lookup(module, plus_sym);
  assign(f, priority_sym, WRAP_REAL(6.0));
  assign(module, symbol_new("+"), f);

  return root;
}

