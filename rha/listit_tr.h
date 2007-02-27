/*
 * listit - An iterator for lists
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef LISTIT_H
#define LISTIT_H

#include "listobject.h"
#include "primtype.h"
#include "list_tr.h"

typedef struct listit_s *listit_tr;
extern primtype_t listit_type;

extern void listit_init();

extern constructor listit_tr  listit_new(list_tr);
extern method      object_t   listit_current(listit_tr);
extern method      void       listit_next(listit_tr);
extern method      bool       listit_done(listit_tr);
extern method      void       listit_replace(listit_tr, object_t);

/*
 * macro for iterating over a list
 *
 * These macros are used as follows:
 *
 *    list_t l;
 *    object_t o;
 *    list_foreach(o, l)
 *      do_something(o);
 *
 */
typedef listobject_iterator_t listit_t; 

// note: the test of the for loop relies somewhat on lazy evaluation.
// This code should even work if the @get function of list.tmpl does
// not check for @done before accessing the iterator.
#define list_foreach(_x, _l)                                   \
  for(listobject_iterator_t li                                 \
	= listobject_iterator(&((list_tr)_l)->list);           \
      (!listobject_done(&li) && ((_x) = listobject_get(&li))); \
      listobject_next(&li))

#endif
