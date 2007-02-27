/*
 * glist - Generic doubly linked LISTs
 *
 * (c) 2005 by Mikio L. Braun
 */

#include "glist.h"
#include <stdlib.h>
#include "utilalloc.h"

typedef struct glistnode node_t;

void glist_init(glist_t *l)
{
  l->len = 0;
  l->first = l->last = 0;
}


void glist_clear(glist_t *l)
{
  node_t *n = l->first, *n2;

  while(n) {
    n2 = n -> next;
    (void)util_free(n);
    n = n2;
  }
}

void glist_append_(glist_t *l, intptr_t data)
{
  node_t *n = util_malloc(sizeof(node_t));
  *n = (node_t) { .prev = 0, .next = 0, .data = data };
  
  if(!l->last) {
    l->first = n; l->last = n;
  }
  else {
    l->last->next = n; n->prev = l->last; l->last = n;
  }
  l->len++;
}




void glist_prepend_(glist_t *l, intptr_t data)
{
  node_t *n = util_malloc(sizeof(node_t));
  *n = (node_t) { .prev = 0, .next = 0, .data = data };
  
  if(!l->first) {
    l->first = n; l->last = n;
  }
  else {
    l->first->prev = n; n->next = l->first; l->first = n;
  }
  l->len++;
}


void glist_appendlist(glist_t *l1, glist_t *l2)
{
  l1->last->next = l2->first;
  l2->first->prev = l1->last;
  l1->last = l2->last;
  l1->len += l2->len;
  l2->first = l2->last = 0;
}


intptr_t glist_remove_(glist_t *l)
     // removes and returns the last element
{
  if(!l->first) // empty glist: nothing to do
    return 0;
  else if(l->first == l->last) { // only one element
    intptr_t v = l->first->data;
    (void)util_free(l->first);
    l->first = l->last = 0;
    l->len--;
    return v;
  }
  else { // more than one element
    node_t *n = l->last;
    intptr_t v = n->data;
    l->last = n->prev;
    l->last->next = 0;
    l->len--;
    (void)util_free(n);
    return v;
  }
}


intptr_t glist_pop_(glist_t *l)
     // removes and returns the first element
{
  if(!l->first) // empty glist: nothing to do
    return 0;
  else if(l->first == l->last) { // only one element
    intptr_t v = l->first->data;
    (void)util_free(l->first);
    l->first = l->last = 0;
    l->len--;
    return v;
  }
  else { // more than one element
    node_t *n = l->first;
    intptr_t v = n->data;
    l->first = n->next;
    l->first->prev = 0;
    l->len--;
    (void)util_free(n);
    return v;
  }
}


intptr_t glist_front_(glist_t *l)
{
  if(!l->first)
    return 0;
  else
    return l->first->data;
}

bool glist_isempty(glist_t *l)
{
  return l->first == 0 && l->last == 0;
}


int glist_length(glist_t *l)
{
  return l->len;
}

void glist_foreach(glist_t *l, glist_proc *p)
{
  for(struct glistnode *n = l->first; n; n = n->next)
    (*p)(n->data);
}

//
// Iterating
//

glist_iterator_t glist_begin(glist_t *l)
{
  glist_iterator_t i;
  i = l->first;
  return i;
}

void glist_next(glist_iterator_t *i)
{
  *i = (*i)->next;
}

bool glist_done(glist_iterator_t *i)
{
  return (*i == 0);
}

intptr_t glist_get_(glist_iterator_t *i)
{
  return (*i)->data;
}


