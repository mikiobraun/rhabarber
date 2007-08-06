/*
 * symtable - the symboltable
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "datatypes.h"

typedef struct symtable *symtable_t;

extern symtable_t symtable_new();
extern object_t symtable_lookup(symtable_t st, symbol_t s);
extern void symtable_assign(symtable_t st, symbol_t s, object_t o);
extern object_t symtable_delete(symtable_t st, symbol_t s);
extern void symtable_constrain(symtable_t st, symbol_t, object_t o);
extern void symtable_print(symtable_t st);
//extern list_tr symtable_tolist(symtable_t st);

#endif
