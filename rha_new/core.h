#ifndef CORE_H
#define CORE_H

#include "rha_types.h"
#include "glist.h"

extern       any_t  split_resolve_and_eval(any_t root, list_t source, string_t context);

extern _rha_ any_t  proxy_fn(any_t this, symbol_t s);

extern _rha_ any_t  create_function(any_t fn_data);

extern       any_t  create_prepattern(any_t theliteral, any_t thetype);
extern _rha_ any_t  create_pattern(any_t theliteral, any_t thetype);
extern _rha_ any_t  create_fn_data(any_t env, tuple_t argnames, 
				      any_t fnbody);
extern _rha_ any_t  create_fn_data_entry(any_t env, tuple_t argnames, 
					    any_t fnbody);
extern _rha_ any_t  fn_fn(any_t env, tuple_t patterns, any_t fnbody);
extern _rha_ any_t  macro_fn(tuple_t argnames, any_t fnbody);

extern _rha_ any_t  if_fn(any_t this, bool_t cond, 
			  any_t then_code, any_t else_code);
extern _rha_ void   return_fn(any_t);
extern _rha_ void   deliver_fn(any_t);
extern _rha_ void   break_fn(any_t);
extern _rha_ void   throw_fn(any_t);
extern _rha_ any_t  while_fn(any_t this, any_t cond, any_t code);
extern _rha_ any_t  for_fn(any_t this, any_t var, 
			   any_t container, any_t code);
extern _rha_ any_t  try_fn(any_t this, any_t tryblock, 
			   symbol_t catchvar, any_t catchblock);

extern _rha_ any_t  colon_fn(any_t a, any_t b);

extern _rha_ any_t  literal(any_t env, list_t parsetree);

extern _rha_ void   tic_fn(void);
extern _rha_ real_t toc_fn(void);
extern _rha_ void   exit_fn(int_t);
extern _rha_ any_t  run_fn(any_t root, string_t fname);

extern _rha_ tuple_t map_fn(any_t this, any_t f, tuple_t t);


#endif
