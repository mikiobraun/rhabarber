/*
 * python_fn - embed the python interpreter in rhabarber
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include "config.h"
#ifdef HAVE_PYTHON

#include <assert.h>
#include "python_fn.h"

#include "alloc.h"
#include "utils.h"
#include "messages.h"
#include "tuple_fn.h"
#include "symbol_fn.h"

static void python_finalizer(GC_PTR obj, GC_PTR cd);

static bool python_started = false;
#define PYTHON_VERSION_LENGTH  256
static char python_version[PYTHON_VERSION_LENGTH];

static any_t python_domain;

static void start_python_if_necessary()
{
  if (!python_started) {
    print("--starting Python\n");
    Py_Initialize();
    //print("  %s\n", Py_GetVersion());

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

void python_init(any_t root, any_t module)
{
  //start_python_if_necessary();
  python_domain = new();
  assign(module, symbol_new("domain"), python_domain);
}


string_t python_getversion()
{
  start_python_if_necessary();

  return python_version;
}


/*
 * Access functions
 */

pyobject_t python_importmodule(string_t name)
{
  start_python_if_necessary();
  PyObject *mod = PyImport_ImportModule(name);
  if(!mod) {
    PyErr_Print();
    rha_error("could not load module\n");
  }
  return mod;
}

void python_runstring(string_t command)
{
  start_python_if_necessary();
  int i = PyRun_SimpleString(command);
  if (i)
    rha_error("(python) command failed");
}

/*
 * Type factory
 */

any_t python_newinteger(int i)
{
  start_python_if_necessary();

  return python_wrap(PyInt_FromLong(i));
}

any_t python_newreal(double d)
{
  start_python_if_necessary();

  return python_wrap(PyFloat_FromDouble(d));
}

any_t python_newtuple(int len)
{
  start_python_if_necessary();

  return python_wrap(PyTuple_New(len));
}

any_t python_newlist(int len)
{
  start_python_if_necessary();

  return python_wrap(PyList_New(len));
}

any_t python_newstring(string_t s)
{
  start_python_if_necessary();

  return python_wrap(PyString_FromString(s));
}

/*
 * Getting Python types
 */
int_t python_getint(pyobject_t o)
{
  start_python_if_necessary();
  if (PyInt_Check(o))
    return PyInt_AsLong(o);
  else
    rha_error("getint() can only be called for integers\n");
  return 0;
}


pyobject_t python_tuple_getitem(pyobject_t o, int i)
{
  start_python_if_necessary();
  if (PyTuple_Check(o))
    return PyTuple_GetItem(o, i);
  else
    rha_error("tuple_getitem() can't be called like this");
  return 0;
}

void python_tuple_setitem(pyobject_t o, int i, pyobject_t v)
{
  start_python_if_necessary();
  if (PyTuple_Check(o)) {
    Py_INCREF(v); // PyTuple_SetItem steals references
    PyTuple_SetItem(o, i, v);
  }
  else
    rha_error("tuple_getitem() can't be called like this");
}

pyobject_t python_list_getitem(pyobject_t o, int i)
{
  start_python_if_necessary();
  if (PyList_Check(o))
    return PyList_GetItem(o, i);
  else
    rha_error("list_getitem() can't be called like this");
  return 0;
}

void python_list_setitem(pyobject_t o, int i, pyobject_t v)
{
  start_python_if_necessary();
  if (PyList_Check(o)) {
    Py_INCREF(v); // PyTuple_SetItem steals references
    PyList_SetItem(o, i, v);
  }
  else
    rha_error("list_getitem() can't be called like this");
}



real_t python_getreal(pyobject_t o)
{
  start_python_if_necessary();
  if (PyFloat_Check(o))
    return PyFloat_AsDouble(o);
  else
    rha_error("getreal() can only be called for reals.\n");
  return 0;

}

string_t python_getstring(pyobject_t o)
{
  start_python_if_necessary();
  if (PyString_Check(o))
    return PyString_AsString(o);
  else
    rha_error("getstring() can only be called for strings.\n");
  return 0;
}


string_t python_to_string(pyobject_t o)
{
  start_python_if_necessary();
  PyObject* str = PyObject_Str(o);

  string_t s = gc_strdup(PyString_AsString(str));
  Py_DECREF(str);
  return s;
}

bool_t python_callable(pyobject_t o)
{
  start_python_if_necessary();
  return PyCallable_Check(o);
}

any_t python_call(tuple_t values)
{
  start_python_if_necessary();
  if(ptype(tuple_get(values, 0)) != PYOBJECT_T) 
    rha_error("(python_call) called for non-python object!");

  pyobject_t fct = UNWRAP_PTR(PYOBJECT_T, tuple_get(values, 0));
  int numargs = tuple_len(values) - 1;

  if (python_callable(fct)) {
    PyObject *args = PyTuple_New(numargs);
    for(int i = 0; i < numargs; i++) {
      any_t a = tuple_get(values, i + 1);
      if (ptype(a) != PYOBJECT_T) {
	Py_DECREF(args);
	rha_error("(python_call) argument %d is not a python object!", i + 1);
      }

      PyObject *arg = UNWRAP_PTR(PYOBJECT_T, a);
      Py_INCREF(arg); // PyTuple_SetItem steals references
      PyTuple_SetItem(args, i, arg);
    }

    PyObject *result = PyObject_Call(fct, args, NULL);

    if (result == NULL) {
      printf("python error: ");PyErr_Print();
      Py_DECREF(args);
      rha_error("Error in python call.\n");
    }
    else {
      Py_DECREF(args);
      if (result == Py_None) {
	Py_DECREF(result);
	return 0;
      }
      else {
	return python_wrap(result);
      }
    }
  }
  else {
    rha_error("Cannot call python object %o.\n", fct);
  }
  return 0;
}

/*
 * lookup a slot in a python object
 * slots added via rhabarber always have precedence
 */
any_t python_lookup(pyobject_t o, symbol_t name)
{
  assert(o);
  start_python_if_necessary();

  if (PyObject_HasAttrString(o, symbol_name(name))) {
    PyObject *retval = 
      PyObject_GetAttrString(o, symbol_name(name));
    if (retval)
      return python_wrap(retval);
    else
      rha_error("How odd, PyObject has attribute %s but won't return it!\n",
		symbol_name(name));
  }
  return 0; // make gcc happy
} 

any_t python_wrap(PyObject *p)
{
  any_t o = WRAP_PTR(PYOBJECT_T, p);

  // register finalizer to decrease pythons counts
  GC_REGISTER_FINALIZER(o, python_finalizer, NULL, NULL, NULL);

  assign(o, domain_sym, python_domain);

  return o;
}


static
void python_finalizer(GC_PTR obj, GC_PTR cd)
{
  any_t p = (any_t)obj;
  assert(ptype(p) == PYOBJECT_T);

  PyObject *po = UNWRAP_PTR(PYOBJECT_T, p);

  print("Calling finalizer for %x, %o\n", (any_t) obj, (any_t) obj);
  Py_DECREF(po);
}

#endif /* HAVE_PYTHON */
