/*
 * pyobject - encapsulating python objects
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef PYOBJECT_H
#define PYOBJECT_H

#include <Python.h>

#include "object.h"
#include "primtype.h"
#include "tuple_tr.h"

typedef struct pyobject_s *pyobject_tr;
extern primtype_t pyobject_type;

extern void pyobject_init();
extern PyObject *pyobject_get(pyobject_tr po);
extern object_t pyobject_new(PyObject *p);

extern method bool     pyobject_callable(pyobject_tr po);

extern method string_t pyobject_to_string(pyobject_tr po);

extern method int      pyobject_getint(pyobject_tr po);
extern method double   pyobject_getreal(pyobject_tr po);
extern method string_t pyobject_getstring(pyobject_tr po);

extern method void pyobject_tuplesetitem(pyobject_tr po, pyobject_tr i, 
					 pyobject_tr o);

#endif
