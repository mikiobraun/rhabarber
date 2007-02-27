#ifndef METHOD_TR_H
#define METHOD_TR_H

#include "object.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "tuplesymbol.h"

typedef struct method_s *method_tr;
extern primtype_t method_type;
extern void        method_init(void);

extern constructor method_tr   method_new(object_t this, object_t fn);
extern method string_t method_to_string(method_tr f);

#endif
