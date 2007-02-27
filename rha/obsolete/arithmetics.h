#ifndef ARITHMETICS_H
#define ARITHMETICS_H

#include "object.h"
#include "symbol_tr.h"

extern void arithmetics_stub_init(object_t root);

extern void arithmetics_overload(symbol_tr op, object_t fct, object_t typ1, object_t typ2);
extern void arithmetics_convert(object_t fct, object_t typ1, object_t typ2);
extern object_t arithmetics_resolve(symbol_tr fct, object_t typ1, object_t typ2);
extern void arithmetics_recomputetable();
extern void arithmetics_dumptables();



#endif
