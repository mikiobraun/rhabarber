#include "list_tr.h"

#include <assert.h>
#include <stdio.h>

#include "listobject.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"
#include "treeobjectobject.h"


#define LIST_IMPLEMENTATION
#ifdef USE_INLINES
#define INLINE inline
#else
#define INLINE
#endif 

#include "list_inline.c"

#define CHECK(x) CHECK_TYPE(list, x)

primtype_t list_type = 0;

void list_init(void)
{
  if (!list_type) {
    list_type = primtype_new("list", sizeof(struct list_s));
    (void) list_new();
  }
}


list_tr list_new()
{
  list_tr l = object_new(list_type);
  listobject_init(&l->list);
  return l;
}


bool list_isempty(list_tr l)
{
  return listobject_isempty(&l->list);
}


list_tr list_clone(list_tr l)
{
  list_tr cl = list_new();
  /* do a deep copy */
  return cl;
}


bool list_bool(list_tr l)
{
  CHECK(l);
  return listobject_length(&l->list) == 0;
}


int list_length(list_tr l)
{
  CHECK(l);
  return listobject_length(&l->list);
}


void list_append(list_tr l, object_t o)
{
  CHECK(l);
  listobject_append(&l->list, o);
}

void list_appendlist(list_tr l1, list_tr l2)
{
  CHECK(l1);
  CHECK(l2);
  listobject_append(&l1->list, &l2->list);
}

void list_prepend(list_tr l, object_t o)
{
  CHECK(l);
  listobject_prepend(&l->list, o);
}


object_t list_pop(list_tr l)
{
  CHECK(l);
  return listobject_pop(&l->list);
}


list_tr list_chop_first(list_tr l, symbol_tr s)
// splits a list into the part before the first appearance of symbol s
// and after it, the symbol itself in the initial list will be lost
{
  list_tr part = list_new();
  object_t head;
  while ((head = list_pop(l))) {
    if (HAS_TYPE(symbol, head))
      if (symbol_equal_symbol(head, s)) 
	break;
    list_append(part, head);
  }
  return part;
}


list_tr list_chop_first_list(list_tr l, list_tr sym_list)
// generalizes list_chop_first() to multiple symbols given in sym_list
{
  list_tr part = list_new();
  object_t head;
  while ((head = list_pop(l))) {
    if (HAS_TYPE(symbol, head)) {
      object_t s;
      list_foreach(s, sym_list)
	if (symbol_equal_symbol(head, s)) 
	  return part;
    }
    list_append(part, head);
  }
  return part;
}


list_tr list_chop_last(list_tr l, symbol_tr s)
// splits a list into the part before the last appearance of symbol s
// and after it, the symbol itself in the initial list will be lost
{
  list_tr part = list_new();
  object_t tail;
  while ((tail = list_remove(l))) {
    if (HAS_TYPE(symbol, tail))
      if (symbol_equal_symbol(tail, s)) 
	break;
    list_prepend(part, tail);
  }
  return part;
}


list_tr list_chop_last_list(list_tr l, list_tr sym_list)
// generalizes list_chop_last() to multiple symbols given in sym_list
{
  list_tr part = list_new();
  object_t tail;
  while ((tail = list_remove(l))) {
    if (HAS_TYPE(symbol, tail)) {
      object_t s;
      list_foreach(s, sym_list)
	if (symbol_equal_symbol(tail, s)) 
	  return part;
    }
    list_prepend(part, tail);
  }
  return part;
}


list_tr list_chop_matching(list_tr l, symbol_tr left, symbol_tr right)
// this is for stuff like 
//    if ... else
//    try ... catch
// where we need match the correct one
// we want: if (true) if (true) 17 else 42  --->   if (true) (if (true) 17 else 42)
// so we need to find our 'else':  e.g. 
// if (true) 17 *else* if (false) 42 else 55 
// if (true) if (false) 42 else 44 *else* 43
{
  int counter = 1;  
  // we have seen one 'if'  (in 'left')
  // an 'else' (in 'right') will decrease the counter
  // once we are at zero we found ours...
  // this is similar to counting brackets
  int llen = list_length(l);
  list_tr part = list_new();
  object_t obj = 0;
  for (int i = 0; i < llen; i++) {
    obj = list_pop(l);
    if (HAS_TYPE(symbol, obj)) {
      if (symbol_equal_symbol(obj, left)) 
	counter++;
      else 
	if (symbol_equal_symbol(obj, right)) 
	  counter--;
      if (counter == 0)
	break;
    }
    list_append(part, obj);
  }
  return part;
}


object_t list_remove(list_tr l)
{
  CHECK(l);
  return listobject_remove(&l->list);
}


object_t list_front(list_tr l)
{
  CHECK(l);
  return listobject_front(&l->list);
}


object_t list_solidify(list_tr l)
{
  if (list_length(l) == 1)
    return list_pop(l);
  else
    return list_to_tuple(l);
}


tuple_tr list_to_tuple(list_tr l)
{
  CHECK(l);
  tuple_tr t = tuple_new(list_length(l));

  listobject_iterator_t it;
  int i;
  for(listobject_begin(&it, &l->list), i = 0;
      !listobject_done(&it);
      listobject_next(&it), i++)
    tuple_set(t, i, listobject_get(&it));   
  return t;
}


string_t list_to_string(list_tr l)
{
  CHECK(l);
  listobject_iterator_t it;
  string_t s = gc_strdup("[");
  listobject_begin(&it, &l->list);
  if (!listobject_done(&it)) {
    s = string_plus_string(s, object_to_string(listobject_get(&it)));
    listobject_next(&it);
  }
  while (!listobject_done(&it)) {
    s = string_plus_string(s, ", ");
    s = string_plus_string(s, object_to_string(listobject_get(&it)));
    listobject_next(&it);
  }
  s = string_plus_string(s, "]");
  return s;
}


/*
 * store (key, value)-pairs of tree in a list
 */
list_tr list_from_tree(treeobjectobject_t t)
{     
  list_tr l = list_new();
  treeobjectobject_iterator_t it;
  for(treeobjectobject_begin(&it, &t); 
      !treeobjectobject_done(&it); 
      treeobjectobject_next(&it)) 
    {
      list_append(l, tuple_make2(treeobjectobject_get_key(&it),
				 treeobjectobject_get_value(&it)));
    }
  return l;
}




#undef CHECK
