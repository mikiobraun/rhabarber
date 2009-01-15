#ifndef CORE_H
#define CORE_H

#include "rha_types.h"
#include "glist.h"

extern       any_t  split_resolve_and_eval(any_t root, list_t source, string_t context);

extern  any_t  proxy_fn(any_t this, symbol_t s);

extern  any_t  create_function(any_t fn_data);

extern       any_t  create_prepattern(any_t theliteral, any_t thetype);
extern  any_t  create_pattern(any_t theliteral, any_t thetype);
extern  any_t  create_fn_data(any_t env, tuple_t argnames, 
				      any_t fnbody);
extern  any_t  create_fn_data_entry(any_t env, tuple_t argnames, 
					    any_t fnbody);
extern  any_t  fn_fn(any_t env, tuple_t patterns, any_t fnbody);
extern  any_t  macro_fn(tuple_t argnames, any_t fnbody);

extern  any_t  if_fn(any_t this, bool cond, 
			  any_t then_code, any_t else_code);
extern  void   return_fn(any_t);
extern  void   deliver_fn(any_t);
extern  void   break_fn(any_t);
extern  void   throw_fn(any_t);
extern  any_t  while_fn(any_t this, any_t cond, any_t code);
extern  any_t  for_fn(any_t this, any_t var, 
			   any_t container, any_t code);
extern  any_t  try_fn(any_t this, any_t tryblock, 
			   symbol_t catchvar, any_t catchblock);

extern  any_t  colon_fn(any_t a, any_t b);

extern  any_t  literal(any_t env, list_t parsetree);

extern  void   tic_fn(void);
extern  real_t toc_fn(void);
extern  void   exit_fn(int);
extern  any_t  run_fn(any_t root, string_t fname);

extern  tuple_t map_fn(any_t this, any_t f, tuple_t t);


#endif
