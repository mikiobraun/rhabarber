#ifndef BUILTIN_TR_H
#define BUILTIN_TR_H

#include "object.h"
#include "symbol_tr.h"
#include "tuple_tr.h"

typedef object_t builtinfct_t(object_t env, tuple_tr in);

typedef struct builtin_s *builtin_tr;
extern primtype_t builtin_type;
extern void       builtin_init(void);

extern builtin_tr builtin_new(builtinfct_t *fun);
extern builtin_tr builtin_new_prule(builtinfct_t *fun, double priority);
extern builtin_tr builtin_new_macro(builtinfct_t *fun);


#define BUILTIN(fn) object_t fn(object_t env, tuple_tr in)
#define ADDBUILTIN(o, s, b) \
   object_assign(o, symbol_new(s), builtin_new(&b))
#define ADDBUILTINMACRO(o, s, b) \
   object_assign(o, symbol_new(s), builtin_new_macro(&b))

#endif
