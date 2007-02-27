#ifndef EXCEPTION_TR_H
#define EXCEPTION_TR_H

#include "primtype.h"
#include "object.h"

typedef struct exception_s *exception_tr;

extern primtype_t exception_type;

extern void exception_init();

extern constructor exception_tr exception_new(string_t);

extern method string_t exception_to_string(exception_tr);

#endif
