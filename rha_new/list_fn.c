#include "list_fn.h"
#include "glist.h"
#include "tuple_fn.h"
#include "alloc.h"
#include "object.h"

list_t list_new()
{
  list_t l = ALLOC_SIZE(sizeof(struct glist));
  glist_init(l);
  return l;
}

list_t list_copy(list_t other)
{
  list_t l = list_new();
  glist_iterator_t it;
  for (it=glist_begin(other); !glist_done(&it); glist_next(&it))
    list_append(l, glist_getp(&it));
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

void list_extend(list_t l, list_t other)
{
  glist_iterator_t it;
  for (it=glist_begin(other); !glist_done(&it); glist_next(&it))
    list_append(l, glist_getp(&it));
}

tuple_t list_to_tuple(list_t l)
{
  tuple_t t = tuple_new(list_len(l));

  glist_iterator_t it;
  int i;
  for (it = glist_begin(l), i=0; !glist_done(&it); glist_next(&it), i++)
    {
      tuple_set(t, i, glist_getp(&it)); 
    }
  return t;
}

object_t list_first(list_t l)
{
  return glist_frontp(l);
}

object_t list_last(list_t l)
{
  return glist_backp(l);
}

object_t list_popfirst(list_t l)
{
  return glist_popp(l);
}

object_t list_poplast(list_t l)
{
  return glist_removep(l);
}

/*
 * Iteration - so far only in C (unfortunately)
 */ 

void list_begin(list_t l, list_it *i)
{
  *i = glist_begin(l);
}

bool list_done(list_it *i)
{
  return glist_done(i);
}

void list_next(list_it *i)
{
  glist_next(i);
}

object_t list_get(list_it *i)
{
  return glist_getp(i);
}


list_t list_chop_first(list_t l, symbol_t s)
// splits a list into the part before the first appearance of symbol s
// and after it, the symbol itself in the initial list will be lost
{
  list_t part = list_new();
  object_t head;
  while ((head = list_popfirst(l))) {
    if (ptype(head) == SYMBOL_T)
      if (s == UNWRAP_SYMBOL(head))
	break;
    list_append(part, head);
  }
  return part;
}


list_t list_chop_first_list(list_t l, glist_t *sym_list)
// generalizes list_chop_first() to multiple symbols given in sym_list
{
  list_t part = list_new();
  object_t head;
  while ((head = list_popfirst(l))) {
    if (ptype(head) == SYMBOL_T)
      if (glist_iselementi(sym_list, UNWRAP_SYMBOL(head)))
	return part;
    list_append(part, head);
  }
  return part;
}


list_t list_chop_last(list_t l, symbol_t s)
// splits a list into the part before the last appearance of symbol s
// and after it, the symbol itself in the initial list will be lost
{
  list_t part = list_new();
  object_t tail;
  while ((tail = list_poplast(l))) {
    if (ptype(tail) == SYMBOL_T)
      if (s == UNWRAP_SYMBOL(tail))
	break;
    list_prepend(part, tail);
  }
  return part;
}


list_t list_chop_last_list(list_t l, glist_t *sym_list)
// generalizes list_chop_last() to multiple symbols given in sym_list
{
  list_t part = list_new();
  object_t tail;
  while ((tail = list_poplast(l))) {
    if (ptype(tail) == SYMBOL_T)
      if (glist_iselementi(sym_list, UNWRAP_SYMBOL(tail)))
	return part;
    list_prepend(part, tail);
  }
  return part;
}
