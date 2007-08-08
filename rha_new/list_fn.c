#include "list_fn.h"
#include "glist.h"
#include "tuple_fn.h"
#include "alloc.h"

list_t list_new()
{
  list_t l = ALLOC_SIZE(sizeof(struct glist));
  glist_init(l);
  return l;
}

int_t list_len(list_t l)
{
  return glist_length(l);
}

void list_append(list_t l, object_t o)
{
  glist_append(l, o);
}

void list_prepend(list_t l, object_t o)
{
  glist_prepend(l, o);
}

tuple_t list_to_tuple(list_t l)
{
  tuple_t t = tuple_new(list_len(l));

  glist_iterator_t it;
  int i;
  for (it = glist_begin(l), i; glist_done(&it); glist_next(&it), i++)
    tuple_set(t, i, glist_getp(&it)); 
}
