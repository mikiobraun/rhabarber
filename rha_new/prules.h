#ifndef PRULES_H
#define PRULES_H

#include "rha_init.h"

extern       void    prules_init(object_t root, object_t module);


extern _rha_ tuple_t plusplus_pr(list_t parsetree);
extern _rha_ tuple_t minusminus_pr(list_t parsetree);
extern _rha_ tuple_t not_pr(list_t parsetree);
extern _rha_ tuple_t divide_pr(list_t parsetree);
extern _rha_ tuple_t times_pr(list_t parsetree);
extern _rha_ tuple_t minus_pr(list_t parsetree);
extern _rha_ tuple_t plus_pr(list_t parsetree);
extern _rha_ tuple_t less_pr(list_t parsetree);
extern _rha_ tuple_t lessequal_pr(list_t parsetree);
extern _rha_ tuple_t greater_pr(list_t parsetree);
extern _rha_ tuple_t greaterequal_pr(list_t parsetree);
extern _rha_ tuple_t equalequal_pr(list_t parsetree);
extern _rha_ tuple_t notequal_pr(list_t parsetree);
extern _rha_ tuple_t and_pr(list_t parsetree);
extern _rha_ tuple_t or_pr(list_t parsetree);
extern _rha_ tuple_t return_pr(list_t parsetree);
extern _rha_ tuple_t deliver_pr(list_t parsetree);
extern _rha_ tuple_t break_pr(list_t parsetree);
extern _rha_ tuple_t throw_pr(list_t parsetree);
extern _rha_ tuple_t run_pr(list_t parsetree);
extern _rha_ tuple_t if_pr(list_t parsetree);
extern _rha_ tuple_t try_pr(list_t parsetree);
extern _rha_ tuple_t while_pr(list_t parsetree);
extern _rha_ tuple_t for_pr(list_t parsetree);
extern _rha_ tuple_t fn_pr(list_t parsetree);
extern _rha_ tuple_t equal_pr(list_t parsetree);
extern _rha_ tuple_t plusequal_pr(list_t parsetree);
extern _rha_ tuple_t minusequal_pr(list_t parsetree);
extern _rha_ tuple_t timesequal_pr(list_t parsetree);
extern _rha_ tuple_t divideequal_pr(list_t parsetree);
extern _rha_ tuple_t quote_pr(list_t parsetree);

#endif
