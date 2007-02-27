/*
 * primtype - encodes the actual implementation type of an object
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */
#ifndef PRIMTYPE_H
#define PRIMTYPE_H

// for size_t which is returned by sizeof
#include <stddef.h>
#include "rhavt.h"

typedef struct primtype *primtype_t;

/* construct a new prim type */
extern primtype_t primtype_new(const char *name, size_t size);

/* retrieve the name of a primtype */
extern const char *primtype_name(primtype_t t);

/* the size of the registered data structure */
extern size_t primtype_size(primtype_t t);

/* the object which is the default parent for objects of this primtype */
extern void *primtype_obj(primtype_t t);

/* modify this object */
extern void primtype_setobj(primtype_t t, void *obj);

/* the virtual table */
extern struct rhavt *primtype_vt(primtype_t t);

/* set the virtual table */
extern void primtype_setvt(primtype_t t, struct rhavt *vt);

#endif
