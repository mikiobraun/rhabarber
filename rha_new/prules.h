#ifndef PRULES_H
#define PRULES_H

#include "rha_init.h"

extern       void    prules_init(object_t root, object_t module);

extern _rha_ tuple_t quote(list_t parsetree);
extern _rha_ tuple_t plus(list_t parsetree);
extern _rha_ tuple_t equal(list_t parsetree);
extern _rha_ tuple_t plusequal(list_t parsetree);
extern _rha_ tuple_t minusequal(list_t parsetree);
extern _rha_ tuple_t timesequal(list_t parsetree);
extern _rha_ tuple_t divideequal(list_t parsetree);

#endif
