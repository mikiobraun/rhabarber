/*
 * python - embed the python interpreter in rhabarber
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include <Python.h>

#include "python_tr.h"
#include "pyobject_tr.h"

#include "utils.h"
#include "messages.h"

#include "object.h"
#include "primtype.h"

struct python_s
{
  RHA_OBJECT;
  /* empty */
};

primtype_t python_type = 0;
python_tr python_domain = 0;


#define CHECK(x) CHECK_TYPE(python, x)

/*
 * We don't want to start python right away (longer startup time).
 * Therefore, we'll waste an extra cycle to check in each function
 * here using the following function.
 */

static bool python_started = false;
#define PYTHON_VERSION_LENGTH  256
static char python_version[PYTHON_VERSION_LENGTH];

static void start_python_if_necessary()
{
  if (!python_started) {
    print("Starting Python ");
    Py_Initialize();
    print("%s\n", Py_GetVersion());

    // extract the version number
    strncpy(python_version, Py_GetVersion(), PYTHON_VERSION_LENGTH);
    python_version[PYTHON_VERSION_LENGTH - 1] = 0;
    char *p = strchr(python_version, ' ');
    if(p != NULL)
      *p = '\0';

    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyString_FromString("."));

    python_started = true;
  }
}

/*
 * Initialize the module
 */

void python_init()
{
  if(!python_type) {
    python_type = primtype_new("python", sizeof(struct python_s));
    python_domain = object_new(python_type);
  }
}


string_t python_getversion(python_tr p)
{
  CHECK(p);
  start_python_if_necessary();

  return python_version;
}


/*
 * Access functions
 */

pyobject_tr python_importmodule(string_t name)
{
  start_python_if_necessary();
  PyObject *mod = PyImport_ImportModule((char*)name);
  if(!mod) {
    PyErr_Print();
    rha_error("could not load module\n");
  }
  return pyobject_new(mod);
}

object_t python_lookup(symbol_tr name)
{
  start_python_if_necessary();
  return 0;
}

object_t python_call(pyobject_tr fct, tuple_tr args)
{
  start_python_if_necessary();

  PyObject *pyargs = PyTuple_New(tuple_length(args));

  for(int i = 0; i < tuple_length(args); i++)
    PyTuple_SetItem(pyargs, i, tuple_get(args, i));

  Py_DECREF(pyargs);
  return 0;
}

object_t python_callmethod(pyobject_tr obj, symbol_tr slot, tuple_tr args)
{
  start_python_if_necessary();

  return 0;
}

/*
 * Type factory
 */

pyobject_tr python_newinteger(int i)
{
  start_python_if_necessary();

  return pyobject_new(PyInt_FromLong(i));
}

pyobject_tr python_newreal(double d)
{
  start_python_if_necessary();

  return pyobject_new(PyFloat_FromDouble(d));
}

pyobject_tr python_newtuple(int len)
{
  start_python_if_necessary();

  return pyobject_new(PyTuple_New(len));
}

pyobject_tr python_newlist()
{
  start_python_if_necessary();

  return pyobject_new(PyList_New(0));
}

pyobject_tr python_newstring(string_t s)
{
  start_python_if_necessary();

  return pyobject_new(PyString_FromString(s));
}


#undef CHECK
