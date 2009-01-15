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

extern  void python_init(any_t, any_t);

extern  any_t python_newinteger(int i);
extern  any_t python_newreal(real_t r);
extern  any_t python_newstring(string_t s);
extern  any_t python_newtuple(int len);
extern  any_t python_newlist(int len);

extern  void      python_tuple_setitem(PyObject *o, int i, PyObject *v);
extern  PyObject *python_tuple_getitem(PyObject *o, int i);

extern  void      python_list_setitem(PyObject *o, int i, PyObject *v);
extern  PyObject *python_list_getitem(PyObject *o, int i);

extern  int       python_getint(PyObject *o);
extern  real_t    python_getreal(PyObject *o);
extern  string_t  python_getstring(PyObject *o);

extern  string_t  python_to_string(PyObject *o);

extern  PyObject *python_importmodule(string_t name);
extern  void      python_runstring(string_t command);

extern  string_t  python_getversion(void);

extern       any_t     python_wrap(PyObject *po);
extern  any_t     python_lookup(PyObject *o, symbol_t name);
extern  bool      python_callable(PyObject *o);
extern  any_t     python_call(tuple_t values);

#endif /* HAVE_PYTHON */

#endif

