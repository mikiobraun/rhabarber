#ifndef PLAIN_TR_H
#define PLAIN_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"

typedef struct plain_s *plain_tr;
extern primtype_t plain_type;
extern void plain_init(void);

extern constructor plain_tr plain_new();
extern method string_t      plain_to_string(plain_tr);
extern method bool          plain_less_plain(plain_tr, plain_tr);
extern method object_t      plain_lookup(plain_tr, symbol_tr);

#endif
