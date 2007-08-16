#ifndef CORE_H
#define CORE_H

#include "rha_types.h"
#include "glist.h"

extern _rha_ object_t  fn_fn(object_t this, tuple_t argnames, object_t fnbody);
extern _rha_ object_t  if_fn(object_t this, bool_t cond, object_t then_code, object_t else_code);
extern _rha_ void      return_fn(object_t);
extern _rha_ void      deliver_fn(object_t);
extern _rha_ void      break_fn(object_t);
extern _rha_ void      throw_fn(object_t);
extern _rha_ object_t  while_fn(object_t this, object_t cond, object_t code);
extern _rha_ object_t  for_fn(object_t this, symbol_t var, object_t container, object_t code);
extern _rha_ object_t  try_fn(object_t this, object_t tryblock, symbol_t catchvar, object_t catchblock);
extern _rha_ void      tic_fn();
extern _rha_ real_t    toc_fn();
extern _rha_ void      exit_fn(int_t);
extern _rha_ object_t  run_fn(object_t root, string_t fname);

extern _rha_ bool_t    and_fn(bool_t, bool_t);
extern _rha_ bool_t    or_fn(bool_t, bool_t);

#endif
