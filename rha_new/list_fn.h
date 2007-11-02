#ifndef LIST_FN_H
#define LIST_FN_H

#include "rha_types.h"
#include "glist.h"

extern _rha_ list_t    list_new(void);
extern _rha_ list_t    list_copy(list_t l);
extern _rha_ int_t     list_len(list_t l);
extern _rha_ list_t    list_append(list_t, any_t o);
extern _rha_ list_t    list_prepend(list_t, any_t o);
extern _rha_ list_t    list_extend(list_t, list_t);

extern _rha_ list_t    list_literal(list_t parsetree);

extern _rha_ tuple_t   list_to_tuple(list_t);
extern _rha_ string_t  list_to_string(list_t);

extern _rha_ any_t     list_first(list_t);
extern _rha_ any_t     list_last(list_t);

extern _rha_ any_t     list_popfirst(list_t);
extern _rha_ any_t     list_poplast(list_t);

extern list_t list_insert_sorted(list_t, any_t, bool_t (*)(any_t, any_t));

// iterator stuff for use in C with MACRO
extern _rha_ list_it_t list_begin(list_t l);
extern _rha_ bool_t    list_done(list_it_t i);
extern _rha_ void      list_next(list_it_t i);
extern _rha_ any_t  list_get(list_it_t i);

// only for parse.c and prules.c
extern list_t list_chop(list_t l, symbol_t s);
extern list_t list_chop_first(list_t l, symbol_t s);
extern list_t list_chop_first_list(list_t l, glist_t *sym_list);
extern list_t list_chop_last(list_t l, symbol_t s);
extern list_t list_chop_last_list(list_t l, glist_t *sym_list);
extern list_t list_chop_matching(list_t l, symbol_t left, symbol_t right);

#endif
