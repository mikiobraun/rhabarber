#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "symbol_tr.h"
#include "list_tr.h"

typedef struct symtable *symtable_t;

extern symtable_t symtable_new();
extern object_t symtable_lookup(symtable_t st, symbol_tr s);
extern void symtable_assign(symtable_t st, symbol_tr s, object_t o);
extern object_t symtable_delete(symtable_t st, symbol_tr s);
extern void symtable_constrain(symtable_t st, symbol_tr, object_t o);
extern void symtable_print(symtable_t st);
extern list_tr symtable_tolist(symtable_t st);

#endif
