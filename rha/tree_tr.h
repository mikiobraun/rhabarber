#ifndef TREE_TR_H
#define TREE_TR_H

#include "object.h"
#include "treeobjectobject.h"

typedef struct tree_s *tree_tr;
extern primtype_t tree_type;
extern void tree_init(void);

extern constructor tree_tr tree_new();

extern method tree_tr tree_clone(tree_tr); // constructs... constructor

extern method int      tree_size(tree_tr);
extern method bool     tree_isempty(tree_tr t);

extern method void     tree_insert(tree_tr t, object_t key, object_t value);
extern method object_t tree_search(tree_tr t, object_t key);
extern method tuple_tr tree_delete(tree_tr t, object_t key);

extern method string_t tree_to_string(tree_tr t);

#endif
