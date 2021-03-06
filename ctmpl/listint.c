// Template generated by maketempl from file list.tmpl
//
// maketempl has been written by Mikio Braun, 2005
// mikio@first.fraunhofer.de
//

#include "listint.h"


#line 59 "list.tmpl"

#include <stdlib.h>

typedef struct nodeint listint_node_t;

void listint_init(listint_t *l)
{
  l->len = 0;
  l->first = l->last = 0;
}


void listint_clear(listint_t *l)
{
  listint_node_t *n = l->first, *n2;

  while(n) {
    n2 = n -> next;
    (void)(n);
    n = n2;
  }
}

void listint_append(listint_t *l, int data)
{
  listint_node_t *n = malloc(sizeof(listint_node_t));
  *n = (listint_node_t) { .prev = 0, .next = 0, .data = data };
  
  if(!l->last) {
    l->first = n; l->last = n;
  }
  else {
    l->last->next = n; n->prev = l->last; l->last = n;
  }
  l->len++;
}


void listint_prepend(listint_t *l, int data)
{
  listint_node_t *n = malloc(sizeof(listint_node_t));
  *n = (listint_node_t) { .prev = 0, .next = 0, .data = data };
  
  if(!l->first) {
    l->first = n; l->last = n;
  }
  else {
    l->first->prev = n; n->next = l->first; l->first = n;
  }
  l->len++;
}


int listint_remove(listint_t *l)
{
  if(!l->first) // empty list: nothing to do
    return 0;
  else if(l->first == l->last) { // only one element
    int v = l->first->data;
    (void)(l->first);
    l->first = l->last = 0;
    l->len--;
    return v;
  }
  else { // more than one element
    listint_node_t *n = l->last;
    int v = n->data;
    l->last = n->prev;
    l->last->next = 0;
    (void)(n);
    return v;
  }
}


int listint_front(listint_t *l)
{
  if(!l->first)
    return 0;
  else
    return l->first->data;
}

bool listint_isempty(listint_t *l)
{
  return l->first == 0 && l->last == 0;
}


int listint_length(listint_t *l)
{
  return l->len;
}

void listint_foreach(listint_t *l, listint_proc *p)
{
  for(struct nodeint *n = l->first; n; n = n->next)
    (*p)(n->data);
}

//
// Iterating
//

void listint_begin(listint_iterator_t *i, listint_t *l)
{
  *i = l->first;
}

void listint_next(listint_iterator_t *i)
{
  *i = (*i)->next;
}

bool listint_done(listint_iterator_t *i)
{
  return (*i == 0);
}

int listint_get(listint_iterator_t *i)
{
  return (*i)->data;
}
