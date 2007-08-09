#ifndef LIST_FN_H
#define LIST_FN_H

#include "rha_types.h"
#include "glist.h"

extern _rha_ list_t   list_new();
extern _rha_ int_t    list_len(list_t l);
extern _rha_ void     list_append(list_t, object_t o);
extern _rha_ void     list_prepend(list_t, object_t o);
extern _rha_ tuple_t  list_to_tuple(list_t);

extern _rha_ object_t list_first(list_t);
extern _rha_ object_t list_last(list_t);

extern _rha_ object_t list_popfirst(list_t);
extern _rha_ object_t list_poplast(list_t);

typedef glist_iterator_t *list_it;

extern void list_begin(list_t l, list_it i);
extern bool list_done(list_it i);
extern void list_next(list_it i);
extern object_t list_get(list_it i);

#endif
