// RHA

#ifndef LIST_H
#define LIST_H

#include "object.h"

/*
 * data structures
 */

struct list
{
  int len;
  struct node *first;
  struct node *last;
};

struct node
{
  struct node *prev;
  struct node *next;
  object_t data;
};

struct listiter
{
  int counter;
  enum { li_before, li_after, li_okay } status;
  struct list *list;
  struct node *current;
};

/*
 * reference section
 */

typedef struct list *list_t;
typedef struct listiter *listiter_t;

extern list_t newlist();
extern void initlist(list_t l);
extern int listsize(list_t l);
extern void pushfrontoflist(list_t l, object_t o);
extern void pushbackoflist(list_t, object_t o);
extern void clearlist(list_t l);
extern object_t listfront(list_t l);

extern void initlistiter(listiter_t li, list_t l);
extern void li_next(listiter_t li);
extern void li_prev(listiter_t li);
extern int li_done(listiter_t li);
extern int li_index(listiter_t li);
extern object_t li_get(listiter_t li);

#endif
