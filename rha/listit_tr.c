/*
 * listit - iterator for list
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include "listit_tr.h"

#include "list_tr.h"
#include "none_tr.h"

struct listit_s
{
  RHA_OBJECT;
  listit_t li;
};

primtype_t listit_type = 0;

#define CHECK(x) CHECK_TYPE(listit, x)

void listit_init()
{
  if(!listit_type) {
    listit_type = primtype_new("listit", sizeof(struct listit_s));
    (void) listit_new(list_new());
  }
}

listit_tr listit_new(list_tr l)
{
  CHECK_TYPE(list, l);
 
  listit_tr li = object_new(listit_type);
  listobject_begin(&li->li, &l->list);
  return li;
}

object_t listit_current(listit_tr li)
{
  CHECK(li);
  return !listit_done(li) 
    ? listobject_get(&li->li)
    : none_obj;
}

void listit_next(listit_tr li)
{
  CHECK(li);
  listobject_next(&li->li);
}

bool listit_done(listit_tr li)
{
  CHECK(li);
  return listobject_done(&li->li);
}

void listit_replace(listit_tr li, object_t v)
{
  CHECK(li);
  if (listit_done(li))
    rha_error("Already at the end, nothing to replace\n");
  listobject_set(&li->li, v);
}

#undef CHECK
