#ifndef CORE_H
#define CORE_H

#include "object.h"
#include "tuple_tr.h"
#include "list_tr.h"
#include "symbol_tr.h"

extern void core_init(void);
extern object_t core_stub_init(void);

extern list_tr path;
extern object_t root;

#define CORE_SYMBOLS_DECLARE
#include "core_symbols.h"
#undef CORE_SYMBOLS_DECLARE

#endif
