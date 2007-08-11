#ifndef PRULES_H
#define PRULES_H

#include "rha_init.h"

extern       object_t prules_init(object_t root, object_t module);

extern _rha_ tuple_t dot(list_t parsetree);
extern _rha_ tuple_t plus(list_t parsetree);



#endif
