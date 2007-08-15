#include "symtable.h"
#include <assert.h>
#include "alloc.h"
#include "list_fn.h"
#include "object.h"
#include "tuple_fn.h"
#include "symbol_fn.h"
#include "string_fn.h"
#include "utils.h"

#include "gtree.h"

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

symtable_t symtable_copy(symtable_t other)
{
  symtable_t s = symtable_new();
  gtree_iterator_t it;
  for (gtree_begin(&it, &other->tree); !gtree_done(&it); gtree_next(&it))
    gtree_insert_(&s->tree, gtree_get_key_(&it), gtree_get_value_(&it));
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

void symtable_print(symtable_t st)
{
  string_t s = gc_strdup("");
  gtree_iterator_t it;
  for (gtree_begin(&it, &st->tree); !gtree_done(&it); gtree_next(&it))
    {
      s = string_append(s, sprint("%s==%o\n", 
				  symbol_name((symbol_t) gtree_get_key_(&it)), 
				  (object_t) gtree_get_value_(&it)));
  }
  fprint(stdout, "%s", s);
}


