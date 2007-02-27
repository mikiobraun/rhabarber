#ifndef LIST_TR_H
#define LIST_TR_H

#include "listobject.h"
#include "tuple_tr.h"
#include "tree_tr.h"

typedef struct list_s *list_tr;
extern primtype_t list_type;
extern void list_init(void);

extern constructor list_tr list_new();

extern method list_tr      list_clone(list_tr); // constructs... constructor
extern method bool         list_isempty(list_tr);
extern method int          list_length(list_tr);
extern method void         list_append(list_tr, object_t);
extern method void         list_appendlist(list_tr, list_tr);
extern method void         list_prepend(list_tr, object_t);
extern method object_t     list_pop(list_tr);
extern method list_tr      list_chop_first(list_tr, symbol_tr);
extern method list_tr      list_chop_first_list(list_tr, list_tr);
extern method list_tr      list_chop_last(list_tr, symbol_tr);
extern method list_tr      list_chop_last_list(list_tr, list_tr);
extern method list_tr      list_chop_matching(list_tr, symbol_tr, symbol_tr);
extern method object_t     list_remove(list_tr);
extern method object_t     list_front(list_tr);
extern method object_t     list_solidify(list_tr);  // warning: rewritten
extern method tuple_tr     list_to_tuple(list_tr);  // equal to the old solidify

extern method string_t     list_to_string(list_tr);

extern list_tr             list_from_tree(treeobjectobject_t);

#include "listit_tr.h"

#ifndef LIST_IMPLEMENTATION
#define INLINE extern inline
#include "list_inline.c"
#undef INLINE
#endif

#endif
