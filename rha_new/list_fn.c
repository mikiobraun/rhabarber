#include "list_fn.h"
#include "glist.h"
#include "tuple_fn.h"
#include "alloc.h"
#include <assert.h>
#include "object.h"
#include "utils.h"
#include "string_fn.h"
#include "messages.h"

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

list_t list_append(list_t l, any_t o)
{
  glist_append(l, o);
  return l;
}

list_t list_prepend(list_t l, any_t o)
{
  glist_prepend(l, o);
  return l;
}

list_t list_extend(list_t l, list_t other)
{
  glist_iterator_t it;
  for (it=glist_begin(other); !glist_done(&it); glist_next(&it))
    list_append(l, glist_getp(&it));
  return l;
}


list_t list_literal(list_t parsetree)
{
  rha_error("not yet");
  assert(1==0);
  return 0; // make gcc happy
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

string_t list_to_string(list_t l)
{
  string_t s = sprint("[");
  list_it_t it;
  int i = 0;
  for(it = list_begin(l); !list_done(it); list_next(it)) {
    if (i++ > 0) s = string_concat(s, ", ");
    s = string_concat(s, to_string(list_get(it)));
  }
  return string_concat(s, "]");
}

any_t list_first(list_t l)
{
  return glist_frontp(l);
}

any_t list_last(list_t l)
{
  return glist_backp(l);
}

any_t list_popfirst(list_t l)
{
  return glist_popp(l);
}

any_t list_poplast(list_t l)
{
  return glist_removep(l);
}

/*
 * Iteration
 */ 

list_it_t list_begin(list_t l)
{
  list_it_t i = ALLOC_SIZE(sizeof(glist_iterator_t));
  *i = glist_begin(l);
  return i;
}

bool list_done(list_it_t i)
{
  return glist_done(i);
}

void list_next(list_it_t i)
{
  glist_next(i);
}

any_t list_get(list_it_t i)
{
  return glist_getp(i);
}


list_t list_chop(list_t l, symbol_t s)
// splits a list into several parts which form the returned list
// if the symbol does not exist, the returned list is empty
// if the provided list contains only the symbol two empty parts are returned
{
  list_t parts = list_new();
  list_t part = list_new();
  any_t head;
  while ((head = list_popfirst(l))) {
    if (ptype(head) == SYMBOL_T)
      if (s == UNWRAP_SYMBOL(head)) {
	// split here
	list_append(parts, WRAP_PTR(LIST_T, part));
	part = list_new();
	continue;
      }
    list_append(part, head);
  }
  list_append(parts, WRAP_PTR(LIST_T, part));
  return parts;
}


list_t list_chop_first(list_t l, symbol_t s)
// splits a list into the part before the first appearance of symbol s
// and after it, the symbol itself in the initial list will be lost
{
  list_t part = list_new();
  any_t head;
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
  any_t head;
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
  any_t tail;
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
  any_t tail;
  while ((tail = list_poplast(l))) {
    if (ptype(tail) == SYMBOL_T)
      if (glist_iselementi(sym_list, UNWRAP_SYMBOL(tail)))
	return part;
    list_prepend(part, tail);
  }
  return part;
}


list_t list_chop_matching(list_t l, symbol_t left, symbol_t right)
// this is for stuff like 
//    if ... else
//    try ... catch
// where we need match the correct one
// we want: if (true) if (true) 17 else 42  --->   if (true) (if (true) 17 else 42)
// so we need to find our 'else':  e.g. 
// if (true) 17 *else* if (false) 42 else 55 
// if (true) if (false) 42 else 44 *else* 43

// example:
//     if (x==0) 17 else 42;
{
  int counter = 1;
  // we have popped off one 'if' from l (in 'left')
  // an 'else' (in 'right') will decrease the counter
  // once we are at zero we found ours...
  // this is similar to counting brackets
  list_t part = list_new();
  any_t head = 0;
  while ((head = list_popfirst(l))) {
    if (ptype(head) == SYMBOL_T) {
      symbol_t head_s = UNWRAP_SYMBOL(head);
      if (head_s == left)
	counter++;
      else if (head_s == right)
	counter--;
    }
    if (counter==0) break;
    list_append(part, head);
  }
  // note that by now:
  // l == "(x==0) 17"
  // part == "42"
  return part;
}


list_t list_insert_sorted(list_t source, any_t entry, 
			  bool_t (*less)(any_t, any_t))
// insert before the smallest larger item in the list
// example:
//  [2, 5, 8]
//  insert 7 with less
//  [2, 5, 7, 8]
{
  list_t sink = list_new();
  list_it_t it = list_begin(source);
  while (!list_done(it)) {
    any_t obj = list_get(it);
    if (!less(obj, entry)) break;
    list_append(sink, obj);
    list_next(it);
  }
  list_append(sink, entry);
  while (!list_done(it)) {
    list_append(sink, list_get(it));
    list_next(it);
  }
  return sink;
}
