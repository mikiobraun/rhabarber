#ifndef SYMBOL_TR_H
#define SYMBOL_TR_H

#include "primtype.h"
#include "object.h"

typedef struct symbol_s *symbol_tr;
extern primtype_t symbol_type;
extern void symbol_init();

extern constructor symbol_tr symbol_new(string_t text);

extern method symbol_tr symbol_clone(symbol_tr s);

extern method string_t symbol_name(symbol_tr s);
extern int symbol_hash(symbol_tr s);

extern operator bool symbol_less_symbol(symbol_tr s1, symbol_tr s2);
extern operator bool symbol_greater_symbol(symbol_tr s1, symbol_tr s2);
extern operator bool symbol_equal_symbol(symbol_tr s1, symbol_tr s2);

extern method string_t symbol_to_string(symbol_tr s);

#define USE_UNIQUEID
//#define USE_STRINGS

#ifndef SYMBOL_IMPLEMENTATION
#define INLINE extern inline
#include "symbol_inline.c"
#undef INLINE
#endif

#endif
