/*
 * pyobject - encapsulating python objects
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifdef HAVE_PYTHON

#include "pyobject_tr.h"

#include <gc/gc.h>
#include "python_tr.h"
#include "string_tr.h"
#include "utils.h"
#include "none_tr.h"
#include "messages.h"
#include "int_tr.h"
#include "real_tr.h"
#include "core.h"
#include "eval.h"
#include "dict_tr.h"


void pyobject_finalizer(GC_PTR obj, GC_PTR cd);

object_t    pyobject_vt_call(object_t env, tuple_tr t);
bool        pyobject_vt_callable(object_t po);
object_t      pyobject_vt_lookup(object_t o, symbol_tr name);

struct pyobject_s
{
  RHA_OBJECT;
  PyObject *object;
};

#define CHECK(x) CHECK_TYPE(pyobject, x)

primtype_t pyobject_type = 0;
object_t pyobject_obj = 0;
struct rhavt pyobject_vt;

void pyobject_init()
{
  if(!pyobject_type) {
    pyobject_type = primtype_new("pyobject", sizeof(struct pyobject_s));
    pyobject_vt = default_vt;
    pyobject_vt.call = pyobject_vt_call;
    pyobject_vt.callable = pyobject_vt_callable;
    pyobject_vt.lookup = pyobject_vt_lookup;
    primtype_setvt(pyobject_type, &pyobject_vt);

    pyobject_obj = pyobject_new(NULL);

    // construct pyobject dictionary
    object_assign(pyobject_obj, symbol_new("prototypes"), dict_new());
  }
}


object_t pyobject_new(PyObject *po)
{
  pyobject_tr p = object_new(pyobject_type);

  // register finalizer to decrease pythons counts
  GC_REGISTER_FINALIZER(p, pyobject_finalizer, NULL, NULL, NULL);

  //printf("Constructing new pyobject %s\n", 
  //       PyString_AsString(PyObject_Str(po)));

  p->object = po;

  if(po && pyobject_obj) {
    PyObject *type = PyObject_Type(po);
    PyObject *typestr = PyObject_Str(type);
    char *t = PyString_AsString(typestr);

    object_t ptdict = object_lookup(pyobject_obj, symbol_new("prototypes"));
    if (ptdict) {
      object_t prototype = dict_search(ptdict, string_new(t));
      
      if(prototype != none_obj) {
	//print("Prototype found for %s\n", t);
	object_snatch(p, prototype);
      }
      //else
	//print("No prototype found for %s\n", t);
    }
    //else
      //print("No dict\n");
    Py_DECREF(typestr);
    Py_DECREF(type);
  }

  object_assign(p, symbol_new("domain"), python_domain);
  return p;
}


void pyobject_finalizer(GC_PTR obj, GC_PTR cd)
{
  if(HAS_TYPE(pyobject, obj)) { // don't assert, just check
    print("Calling finalizer for %x, %o\n", _O obj, _O obj);
    Py_DECREF(pyobject_get(obj));
  }
}


string_t pyobject_to_string(pyobject_tr po)
{
  CHECK(po);
  PyObject* str = PyObject_Str(pyobject_get(po));

  string_t s = gc_strdup(PyString_AsString(str));
  Py_DECREF(str);
  return s;
}


PyObject *pyobject_get(pyobject_tr po)
{
  CHECK(po);
  return po->object;
}




int pyobject_getint(pyobject_tr po)
{
  CHECK(po);
  if (PyInt_Check(po->object))
    return PyInt_AsLong(po->object);
  else
    rha_error("getint() can only be called for integers\n");
  return 0;
}


double pyobject_getreal(pyobject_tr po)
{
  CHECK(po);
  if (PyFloat_Check(po->object))
    return PyFloat_AsDouble(po->object);
  else
    rha_error("getreal() can only be called for reals.\n");
  return 0;
}


string_t pyobject_getstring(pyobject_tr po)
{
  CHECK(po);
  if (PyString_Check(po->object))
    return PyString_AsString(po->object);
  else
    rha_error("getstring() can only be called for strings.\n");
  return 0;
}


bool pyobject_callable(pyobject_tr po)
{
  CHECK(po);
  return pyobject_vt_callable(po);
}

/***********************************************************************
 *
 * Virtual table functions
 *
 **********************************************************************/


bool pyobject_vt_callable(object_t o)
{
  pyobject_tr po = o;
  CHECK(po);
  return PyCallable_Check(po->object);
}


object_t pyobject_vt_call(object_t env, tuple_tr t)
{
  assert(HAS_TYPE(tuple, t));

  pyobject_tr fct = tuple_get(t, 0);
  int numargs = tuple_length(t) - 1;

  CHECK(fct);
  if (pyobject_vt_callable(fct)) {

    t = eval_fctcall_hook(env, t);

    PyObject *args = PyTuple_New(numargs);
    for(int i = 0; i < numargs; i++) {
      pyobject_tr arg = tuple_get(t, i+1);
      if (!HAS_TYPE(pyobject, arg)) {
	Py_DECREF(args);
	rha_error("Argument %d is not a python object. "
		  "(Maybe import domains first)\n", i+1);
      }
      //printf("arg %d = %s\n", i, pyobject_to_string(arg));
      Py_INCREF(pyobject_get(arg)); // PyTuple_SetItem steals references
      PyTuple_SetItem(args, i, pyobject_get(arg));
    }

    PyObject *result = PyObject_Call(pyobject_get(fct), args, NULL);

    if (result == NULL) {
      PyErr_Print();
      Py_DECREF(args);
      rha_error("Error in python call.\n");
    }
    else {
      Py_DECREF(args);
      if (result == Py_None) {
	Py_DECREF(result);
	return none_obj;
      }
      else {
	return pyobject_new(result);
      }
    }
  }
  else {
    //print("Looking up %o in %o\n", call_sym, fct);
    object_t call = rha_lookup(fct, call_sym);
    if(call)
      return callcall(env, call, t);
    else
      rha_error("Cannot call python object %o.\n", fct);
  }
  return none_obj;
}


/*
 * lookup a slot in a python object
 * slots added via rhabarber always have precedence
 */
object_t pyobject_vt_lookup(object_t o, symbol_tr name)
{
  CHECK(o);
  object_t retval = object_lookup(o, name);
  if (retval) {
    return retval;
  }
  else {
    pyobject_tr po = o;
    if(po->object) {
      if (PyObject_HasAttrString(pyobject_get(po), (char*)symbol_name(name))) {
	PyObject *retval = 
	  PyObject_GetAttrString(pyobject_get(po), (char*)symbol_name(name));
	if (retval)
	  return pyobject_new(retval);
	else
	  rha_error("How odd, PyObject has attribute %s but won't return it!\n",
		    symbol_name(name));
      }
      else
	if(object_parent(o)) 
	  return rha_lookup(object_parent(o), name);
    }
  }
  return 0;
} 

void pyobject_tuplesetitem(pyobject_tr po, pyobject_tr idx, 
			   pyobject_tr o)
{
  CHECK(po);
  CHECK(idx);
  CHECK(o);

  if(!PyTuple_Check(po->object)
     || !PyInt_Check(idx->object))
    rha_error("pyobject.tuplesetitem excepts tuple, number, object");

  int i = PyInt_AsLong(idx->object);

  Py_INCREF(o->object); // SetItem steals references
  PyTuple_SetItem(po->object, i, o->object);
}

#endif /* HAVE_PYTHON */
