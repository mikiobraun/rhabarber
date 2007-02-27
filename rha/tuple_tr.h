#ifndef TUPLE_TR_H
#define TUPLE_TR_H

#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"
#include "tupleobject.h"

typedef struct tuple_s *tuple_tr;
extern primtype_t tuple_type;
extern void tuple_init(void);

extern constructor tuple_tr tuple_new(int length);
extern tuple_tr tuple_make_unstable(object_t, ...);
extern tuple_tr tuple_make(int, ...);
extern tuple_tr tuple_make1(object_t);
extern tuple_tr tuple_make2(object_t, object_t);
extern tuple_tr tuple_make3(object_t, object_t, object_t);
extern tuple_tr tuple_shiftfirst(tuple_tr);

extern method tuple_tr tuple_copy(tuple_tr t);

extern method int tuple_length(tuple_tr t);
extern method object_t tuple_get(tuple_tr t, int i);
extern method void tuple_set(tuple_tr t, int i, object_t o);

extern operator bool tuple_less_tuple(tuple_tr t1, tuple_tr t2);

extern method string_t tuple_to_string(tuple_tr t);
extern method object_t tuple_to_list(tuple_tr);

#ifndef TUPLE_IMPLEMENTATION
#define INLINE extern inline
#include "tuple_inline.c"
#undef INLINE
#endif

#endif
