#include "symtable.h"
#include <assert.h>
#include "alloc.h"

#define USE_TREE
//#define USE_HASH

struct stentry {
  //  object_t constraint;
  object_t obj;
};

#ifdef USE_HASH
#include "hashsymbolobject.h"

struct symtable
{
  hashsymbolobject_t hash;
};

symtable_t symtable_new()
{
  symtable_t s = ALLOC(symtable);
  
  hashsymbolobject_init(&s->hash);
  
  return s;
}

object_t symtable_lookup(symtable_t st, symbol_tr s)
{
  return hashsymbolobject_lookup(&st->hash, s);
}

void symtable_assign(symtable_t st, symbol_tr s, object_t o)
{
  hashsymbolobject_assign(&st->hash, s, o); 
}

object_t symtable_delete(symtable_t st, symbol_tr s)
{
  assert(0); /* never reach this point! */
  return 0;
}

list_tr symtable_tolist(symtable_t st)
{
  list_tr l = list_new();

  hashsymbolobject_iterator_t it;
  for(hashsymbolobject_begin(&it, &st->hash);
      !hashsymbolobject_done(&it);
      hashsymbolobject_next(&it)) {
    list_append(l, tuple_make2(hashsymbolobject_get_key(&it),
			       hashsymbolobject_get_value(&it)));
  }
  return l;
}

#elif defined(USE_TREE)
#include "treesymbolobject.h"

struct symtable
{
  treesymbolobject_t tree;
};


symtable_t symtable_new()
{
  symtable_t s = ALLOC(symtable);
  
  treesymbolobject_init(&s->tree);
  
  return s;
}

object_t symtable_lookup(symtable_t st, symbol_tr s)
{
  return treesymbolobject_search(&st->tree, s);
}

void symtable_assign(symtable_t st, symbol_tr s, object_t o)
{
  treesymbolobject_insert(&st->tree, s, o); 
}

object_t symtable_delete(symtable_t st, symbol_tr s)
{
  return treesymbolobject_delete(&st->tree, s);
}

void symtable_print(symtable_t st)
{
  treesymbolobject_iterator_t i;
  for (treesymbolobject_begin(&i, &(st->tree));
       !treesymbolobject_done(&i);
       treesymbolobject_next(&i))
    printf("%s\n", symbol_name(treesymbolobject_get_key(&i)));
}

list_tr symtable_tolist(symtable_t st)
{
  list_tr l = list_new();

  treesymbolobject_iterator_t i;
  for (treesymbolobject_begin(&i, &(st->tree));
       !treesymbolobject_done(&i);
       treesymbolobject_next(&i)) {
    tuple_tr t = tuple_new(2);
    tuple_set(t, 0, treesymbolobject_get_key(&i));
    tuple_set(t, 1, treesymbolobject_get_value(&i));
    list_append(l, t);
  }
  return l;
}

#else
#error "Please specify which dictionary structure to use!"
#endif

