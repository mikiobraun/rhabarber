/*
 * python - embed the python interpreter in rhabarber
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifdef HAVE_PYTHON

#ifndef PYTHON_H
#define PYTHON_H

#include "pyobject_tr.h"

#include "object.h"
#include "symbol_tr.h"
#include "tuple_tr.h"

typedef struct python_s *python_tr;
extern primtype_t python_type;

extern python_tr python_domain;

extern void python_init();

extern object_t python_lookup(symbol_tr name);
extern object_t python_call(pyobject_tr fct, tuple_tr args);
extern object_t python_callmethod(pyobject_tr obj, symbol_tr slot, tuple_tr args);


extern constructor pyobject_tr python_newinteger(int i);
extern constructor pyobject_tr python_newreal(double r);
extern constructor pyobject_tr python_newstring(string_t s);
extern constructor pyobject_tr python_newtuple(int len);
extern constructor pyobject_tr python_newlist();

extern constructor pyobject_tr python_importmodule(string_t name);

extern method string_t python_getversion(python_tr p);

#endif

#endif /* HAVE_PYTHON */
