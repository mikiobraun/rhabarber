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

extern _rha_ any_t python_newinteger(int i);
extern _rha_ any_t python_newreal(real_t r);
extern _rha_ any_t python_newstring(string_t s);
extern _rha_ any_t python_newtuple(int len);
extern _rha_ any_t python_newlist(int len);

extern _rha_ void      python_tuple_setitem(PyObject *o, int i, PyObject *v);
extern _rha_ PyObject *python_tuple_getitem(PyObject *o, int i);

extern _rha_ void      python_list_setitem(PyObject *o, int i, PyObject *v);
extern _rha_ PyObject *python_list_getitem(PyObject *o, int i);

extern _rha_ int       python_getint(PyObject *o);
extern _rha_ real_t    python_getreal(PyObject *o);
extern _rha_ string_t  python_getstring(PyObject *o);

extern _rha_ string_t  python_to_string(PyObject *o);

extern _rha_ PyObject *python_importmodule(string_t name);
extern _rha_ void      python_runstring(string_t command);

extern _rha_ string_t  python_getversion(void);

extern       any_t     python_wrap(PyObject *po);
extern _rha_ any_t     python_lookup(PyObject *o, symbol_t name);
extern _rha_ bool      python_callable(PyObject *o);
extern _rha_ any_t     python_call(tuple_t values);

#endif /* HAVE_PYTHON */

#endif

