#ifndef OVERLOADED_TR_H
#define OVERLOADED_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"
#include "tuple_tr.h"

typedef struct overloaded_s *overloaded_tr;
extern primtype_t overloaded_type;
extern void overloaded_init(void);

extern constructor overloaded_tr overloaded_new();
extern method void overloaded_add(overloaded_tr ov, object_t f, 
				  tuple_tr signature);

#endif
