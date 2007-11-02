/*
 * python_fn - embed the python interpreter in rhabarber
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */


#ifndef PYTHON_FN_H
#define PYTHON_FN_H

#include "config.h"
#ifdef HAVE_PYTHON

#include "rha_types.h"

extern _rha_ void python_init(any_t, any_t);

extern _rha_ any_t python_newinteger(int_t i);
extern _rha_ any_t python_newreal(real_t r);
extern _rha_ any_t python_newstring(string_t s);
extern _rha_ any_t python_newtuple(int_t len);
extern _rha_ any_t python_newlist(int_t len);

extern _rha_ void       python_tuple_setitem(pyobject_t o, int_t i, pyobject_t v);
extern _rha_ pyobject_t python_tuple_getitem(pyobject_t o, int_t i);

extern _rha_ void       python_list_setitem(pyobject_t o, int_t i, pyobject_t v);
extern _rha_ pyobject_t python_list_getitem(pyobject_t o, int_t i);

extern _rha_ int_t      python_getint(pyobject_t o);
extern _rha_ real_t     python_getreal(pyobject_t o);
extern _rha_ string_t   python_getstring(pyobject_t o);

extern _rha_ string_t   python_to_string(pyobject_t o);

extern _rha_ pyobject_t python_importmodule(string_t name);
extern _rha_ void python_runstring(string_t command);

extern _rha_ string_t python_getversion(void);

extern       any_t  python_wrap(PyObject *po);
extern _rha_ any_t  python_lookup(pyobject_t o, symbol_t name);
extern _rha_ bool_t python_callable(pyobject_t o);
extern _rha_ any_t  python_call(tuple_t values);

#endif /* HAVE_PYTHON */

#endif

