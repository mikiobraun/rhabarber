#include "tree_tr.h"

#include <assert.h>
#include <stdio.h>

#include "treeobjectobject.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"

struct tree_s
{
  RHA_OBJECT;
  treeobjectobject_t tree;
  object_t less;
};

#define CHECK(x) CHECK_TYPE(tree, x)

primtype_t tree_type = 0;

void tree_init(void)
{
  if (!tree_type) {
    tree_type = primtype_new("tree", sizeof(struct tree_s), tree_obj);
    (void) tree_new(plan_new());
  }
}

tree_tr tree_new(object_t less)
{
  tree_tr t = object_new(tree_type);
  treeobjectobject_init(&t->tree);
  t->less = less;
  return t;
}

int tree_size(tree_tr t)
{
  CHECK(t);
  return treeobjectobject_size(&t->tree);
}


bool tree_isempty(tree_tr t)
{
  CHECK(t);
  return treeobjectobject_isempty(&t->tree);
}


void tree_insert(tree_tr t, object_t key, object_t value)
{
  CHECK(t);
  treeobjectobject_insert(&t->tree, key, value);
}


object_t tree_search(tree_tr t, object_t key)
{
  CHECK(t);
  return treeobjectobject_search(&t->tree, key);
}



#undef CHECK

