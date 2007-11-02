#ifndef PRULES_H
#define PRULES_H

#include "rha_init.h"

extern       void     prules_init(any_t root, any_t module);

// helper
extern       list_t split_by_semicolon(list_t parsetree);

extern       bool_t   is_symbol(symbol_t a_symbol, any_t expr);
extern       any_t quoted(any_t obj);
extern       bool_t   is_marked_list(symbol_t mark, any_t obj);
extern       list_t   split_rounded_list_obj(any_t list_obj);

extern       glist_t  assign_sym_list; // a list with = += -= *= /=
extern       glist_t  cmp_sym_list;    // a list with < <= > >= == !=
extern       glist_t  sec_sym_list;    // a list with 'else', 'catch'

// function to define common prules
extern _rha_ any_t prule_new_infix(void);
extern _rha_ any_t prule_new_prefix(void);
extern _rha_ any_t prule_new_postfix(void);
extern _rha_ any_t prule_new_freefix(void);

// predefined prules
extern _rha_ tuple_t curlied_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t squared_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t apostrophe_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t plusplus_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t minusminus_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t not_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t divide_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t times_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t dotdivide_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t dottimes_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t prefix_minus_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t minus_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t plus_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t colon_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t in_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t less_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t lessequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t greater_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t greaterequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t equalequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t notequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t and_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t or_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t return_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t deliver_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t break_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t throw_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t if_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t try_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t while_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t for_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t fn_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t macro_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t equal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t plusequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t minusequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t timesequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t divideequal_pr(any_t env, list_t parsetree);
extern _rha_ tuple_t quote_pr(any_t env, list_t parsetree);

#endif
