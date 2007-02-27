/*
 * primtype - encodes the actual implementation type of an object
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */
#include "primtype.h"

#include <string.h>
#include <stddef.h>
#include "alloc.h"
#include "debug.h"
#include "object.h"
#include "rhavt.h"

struct primtype
{
  char *name;       // name of the primtype
  size_t size;      // the return type of sizeof
  object_t obj;     // the object which represents this primtype
  struct rhavt *vt; // the virtual table (see rhavt)
};


primtype_t primtype_new(const char *name, size_t size)
{
  primtype_t t = ALLOC(primtype);
  t->name = gc_strdup(name);
  t->size = size;
  t->obj = 0;
  t->vt = &default_vt;
  printdebug("primtype: %s created (%p)\n", t->name, (void *) t);
  return t;
}


const char *primtype_name(primtype_t t)
{
  return t->name;
}


size_t primtype_size(primtype_t t)
{
  return t->size;
}


object_t primtype_obj(primtype_t t)
{
  return t->obj;
}


void primtype_setobj(primtype_t t, object_t obj)
{
  t->obj = obj;
}


struct rhavt *primtype_vt(primtype_t t)
{
  return t->vt;
}


void primtype_setvt(primtype_t t, struct rhavt *vt)
{
  t->vt = vt;
}
