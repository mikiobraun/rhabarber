#include "alloc.h"
#include "list.h"

/*
 * list
 */

list_t newlist()
{
  list_t l = ALLOC(list);
  initlist(l);
  return l;
}


void initlist(list_t l)
{
  l->len = 0;
  l->first = 0;
  l->last = 0;
}


int listsize(list_t l)
{
  return l->len;
}


void pushfrontoflist(list_t l, object_t o)
{
  struct node *n = ALLOC(node);
  n->prev = 0; n->next = 0; n->data = o;
  if(l->len == 0) {
    l->first = l->last = n;
  }
  else {
    l->first->prev = n;
    n->next = l->first;
    l->first = n;
  }
  l->len++;
}


void pushbackoflist(list_t l, object_t o)
{
  struct node *n = ALLOC(node);
  n->prev = 0; n->next = 0; n->data = o;
  if(l->len == 0) {
    l->first = l->last = n;
  }
  else {
    l->last->next = n;
    n->prev = l->last;
    l->last = n;
  }
  l->len++;
}


void clearlist(list_t l)
{
  l->len = 0;
  l->first = 0;
  l->last = 0;
}


object_t listfront(list_t l)
{
  if(!l->len)
    return 0;
  else
    return l->first->data;
}

/*
 * list iterators
 */


void initlistiter(listiter_t li, list_t l)
{
  li->counter = 0;
  li->status = li_okay;
  li->list = l;
  li->current = l->first;
}


void li_next(listiter_t li)
{
  if(li->status != li_after) {
    li->counter++;
    li->current = li->current->next;
    if(!li->current) li->status = li_after;
    else li->status = li_okay;
  }
}


void li_prev(listiter_t li)
{
  if(li->status != li_before) {
    li->counter--;
    li->current = li->current->prev;
    if(!li->current) li->status = li_before;
    else li->status = li_okay;
  }
}


int li_done(listiter_t li)
{
  return li->status != li_okay;
}


int li_index(listiter_t li)
{
  return li->counter;
}


object_t li_get(listiter_t li)
{
  return li->current->data;
}


