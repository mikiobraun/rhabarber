#ifndef PRULES_H
#define PRULES_H

#include "rha_init.h"

extern       void    prules_init(object_t root, object_t module);

// function to define common prules
extern _rha_ object_t prule_new_infix();
extern _rha_ object_t prule_new_prefix();
extern _rha_ object_t prule_new_postfix();
extern _rha_ object_t prule_new_freefix();

// predefined prules
extern _rha_ tuple_t plusplus_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t minusminus_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t not_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t divide_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t times_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t prefix_minus_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t minus_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t plus_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t in_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t less_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t lessequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t greater_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t greaterequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t equalequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t notequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t and_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t or_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t print_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t return_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t deliver_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t break_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t throw_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t if_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t try_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t while_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t for_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t fn_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t equal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t plusequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t minusequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t timesequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t divideequal_pr(object_t env, list_t parsetree);
extern _rha_ tuple_t quote_pr(object_t env, list_t parsetree);

#endif
