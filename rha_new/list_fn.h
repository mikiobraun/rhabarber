#ifndef LIST_FN_H
#define LIST_FN_H

#include "rha_types.h"
#include "glist.h"

extern _rha_ list_t   list_new();
extern _rha_ list_t   list_copy(list_t l);
extern _rha_ int_t    list_len(list_t l);
extern _rha_ void     list_append(list_t, object_t o);
extern _rha_ void     list_prepend(list_t, object_t o);
extern _rha_ void     list_extend(list_t, list_t);

extern _rha_ list_t   list_literal(list_t parsetree);

extern _rha_ tuple_t  list_to_tuple(list_t);

extern _rha_ object_t list_first(list_t);
extern _rha_ object_t list_last(list_t);

extern _rha_ object_t list_popfirst(list_t);
extern _rha_ object_t list_poplast(list_t);

// iterator stuff for use in C with MACRO
typedef glist_iterator_t list_it;
extern void list_begin(list_t l, list_it *i);
extern bool list_done(list_it *i);
extern void list_next(list_it *i);
extern object_t list_get(list_it *i);

// only for parse.c and prules.c
extern list_t list_chop_first(list_t l, symbol_t s);
extern list_t list_chop_first_list(list_t l, glist_t *sym_list);
extern list_t list_chop_last(list_t l, symbol_t s);
extern list_t list_chop_last_list(list_t l, glist_t *sym_list);
extern list_t list_chop_matching(list_t l, symbol_t left, symbol_t right);

#endif
