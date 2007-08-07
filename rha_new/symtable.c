#include "symtable.h"
#include <assert.h>
#include "alloc.h"

#include "util/gtree.h"

struct symtable
{
  struct gtree tree;
};

static bool symtable_less(intptr_t a, intptr_t b)
{
  return a < b;
}

symtable_t symtable_new()
{
  symtable_t s = ALLOC(symtable);
  
  gtree_init(&s->tree, symtable_less);
  
  return s;
}

object_t symtable_lookup(symtable_t st, symbol_t s)
{
  return gtree_searchp(&st->tree, s);
}

void symtable_assign(symtable_t st, symbol_t s, object_t o)
{
  gtree_insert(&st->tree, s, o); 
}

object_t symtable_delete(symtable_t st, symbol_t s)
{
  return gtree_deletep(&st->tree, s);
}



