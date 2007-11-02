/*
 * matlab - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#include "matlab_tr.h"

#include <string.h>

#include <matrix.h>
#include <engine.h>

#include "object.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "utils.h"
#include "messages.h"
#include "none_tr.h"

#include "mxarray_tr.h"
#include "mxfunc_tr.h"

struct matlab_s
{
  RHA_OBJECT;
  Engine *eng;
  int varid_counter;
  char *outputbuffer;
};

#define CHECK(x) CHECK_TYPE(matlab, x)

primtype_t matlab_type = 0;
matlab_tr matlab_domain = 0;
struct rhavt matlab_vt;

#define MATLAB_OUTPUT_BUFSIZE 65536


/*------------------------------------------------------------
 * Forward declarations of static functions
 */ 
static char *varid_asstring(char *, varid_t v);
static void start_matlab_if_necessary(void);

static object_t matlab_vt_lookup(object_t o, symbol_tr s);

/************************************************************
 *
 * Construction
 *
 ************************************************************/

void matlab_init(void)
{
  if (!matlab_type) {
    matlab_type = primtype_new("matlab", sizeof(struct matlab_s));
    matlab_vt = default_vt;
    matlab_vt.lookup = matlab_vt_lookup;
    primtype_setvt(matlab_type, &matlab_vt);
    /* add more initialization here */
    matlab_domain = matlab_new();

  }
}

matlab_tr matlab_new(void)
{
  matlab_tr newmatlab = object_new(matlab_type);
  newmatlab->eng = 0;
  return newmatlab;
}


static void start_matlab_if_necessary(void)
{
  if(!matlab_domain->eng) {
    print("Starting Matlab engine...\n");
#if !defined(_WIN32)
    matlab_domain->eng = engOpen("matlab -nosplash");
#else
    matlab_domain->eng = engOpen(NULL);
#endif
    if(!matlab_domain->eng) {
      rha_error("Cannot start MATLAB engine!\n");
    }

    matlab_domain->outputbuffer = ALLOC_RAW(MATLAB_OUTPUT_BUFSIZE*sizeof(char));

    engOutputBuffer(matlab_domain->eng, matlab_domain->outputbuffer, 
		    MATLAB_OUTPUT_BUFSIZE);

    matlab_domain->varid_counter = 0;
    print("done.\n");
  }
}

/************************************************************
 *
 * Access methods
 *
 ************************************************************/

void matlab_evalstring(string_t msg)
{
#ifdef MATLAB_R12
  static char *errorprefix = "??? ";
#elif MATLAB_R13
  static char *errorprefix = ">>\n??? ";
#else
#error "Unknown MATLAB Version"
#endif
  int eplen = strlen(errorprefix);

  start_matlab_if_necessary();

  print("Evaluating \"%s\"\n", msg);
  engEvalString(matlab_domain->eng, msg);
  //print("%s", matlab_domain->outputbuffer);

  if(*matlab_domain->outputbuffer) {
    if(!strncmp(matlab_domain->outputbuffer, errorprefix, eplen))
      rha_error("Matlab error\n%s", matlab_domain->outputbuffer + eplen);
    print("%s", matlab_domain->outputbuffer + 3);
  }
}

varid_t matlab_newvarid(void)
{
  return ++matlab_domain->varid_counter;
}


void matlab_clear(varid_t vi)
{
  start_matlab_if_necessary();
  /* string constants are usually not writable */
  static char buffer[] = { 'c', 'l', 'e', 'a', 'r', ' ', 'R', 
			   '0', '1', '2', '3', '4', '5', '6', '7',
			   ';', '\0' };
  varid_asstring(buffer + 7, vi);
  matlab_evalstring(buffer);
}


void matlab_put(varid_t vi, mxArray *a)
{
  static char buffer[] = { 'R', '0', '1', '2', '3', '4', '5', '6', '7',
			   '\0' };
  varid_asstring(buffer + 1, vi);
#ifdef MATLAB_R12
  mxSetName(a, buffer);
  engPutArray(matlab_domain->eng, a);
#elif defined(MATLAB_R13)
  engPutVariable(matlab_domain->eng, buffer, a);
#else
#  error "Unknown matlab version"
#endif
}


mxArray *matlab_get(varid_t vi)
{
  static char buffer[] = { 'R', '0', '1', '2', '3', '4', '5', '6', '7',
			   '\0' };
  varid_asstring(buffer + 1, vi);
#if defined(MATLAB_R12)
  return engGetArray(matlab_domain->eng, buffer);
#elif defined(MATLAB_R13)
  return engGetVariable(matlab_domain->eng, buffer);
#else
#  error "Unknown matlab version"
#endif
}

string_t matlab_varidname(varid_t vi)
{
  static char buffer[] = { 'R', '0', '1', '2', '3', '4', '5', '6', '7',
			   '\0' };
  varid_asstring(buffer + 1, vi);
  return gc_strdup(buffer);
}

/*************************************************************
 *
 * Object constructors.
 *
 ************************************************************/

object_t matlab_newinteger(int i)
{
  start_matlab_if_necessary();
  return mxarray_new(mxCreateScalarDouble(i), 0);
}

object_t matlab_newreal(double r)
{
  start_matlab_if_necessary();
  return mxarray_new(mxCreateScalarDouble(r), 0);
}

object_t matlab_newbool(bool b)
{
  start_matlab_if_necessary();
#ifdef MATLAB_R13
  return mxarray_new(mxCreateLogicalScalar(b), 0);
#else
  return mxarray_new(mxCreateScalarDouble(b), 0);
#endif
}

object_t matlab_newstring(string_t s)
{
  start_matlab_if_necessary();
  return mxarray_new(mxCreateString(s), 0);
}

object_t matlab_newmatrix(int rows, int cols)
{
  start_matlab_if_necessary();
  return mxarray_new(mxCreateDoubleMatrix(rows, cols, mxREAL), 0);
}

object_t matlab_call(string_t fn, int outargs, object_t args)
{
  string_t cmd;
  int i;

  if (!HAS_TYPE(none, args)) {
    if (!HAS_TYPE(tuple, args))
      args = tuple_make1(args);

    for(i = 0; i < tuple_length(args); i++) {
      if(!HAS_TYPE(mxarray, tuple_get(args, i))) {
	rha_error("matlab.call: arguments must be mxarrays");
      }
      mxarray_putobj(tuple_get(args, i));
    }
  }

  if (!outargs) {
    // just construct the calling tuple
    if(args != none_obj) {
      cmd = sprint("%s(", fn);
      for(i = 0; i < tuple_length(args) - 1; i++)
	cmd = sprint("%sR%08x, ", cmd, mxarray_varid(tuple_get(args, i)));
      cmd = sprint("%sR%08x);", cmd, mxarray_varid(tuple_get(args, i)));
      matlab_evalstring(cmd);
    }
    else {
      cmd = sprint("%s;", fn);
      matlab_evalstring(cmd);
    }
    return none_obj;
  }
  if (outargs == 1) {
    // one output argument
    varid_t out = matlab_newvarid();
    if(args != none_obj) {
      cmd = sprint("R%08x = %s(", out, fn);
      for(i = 0; i < tuple_length(args) - 1; i++)
	cmd = sprint("%sR%08x, ", cmd, mxarray_varid(tuple_get(args, i)));
      cmd = sprint("%sR%08x);", cmd, mxarray_varid(tuple_get(args, i)));
    }
    else {
      cmd = sprint("R%08x = %s;", out, fn);
    }
    matlab_evalstring(cmd);
    return mxarray_new(0, out);
  }
  else {
    // multiple output arguments, (returned as tuple)
    tuple_tr out = tuple_new(outargs);
    for(int i = 0; i < outargs; i++)
      tuple_set(out, i, mxarray_new(0, matlab_newvarid()));
    cmd = sprint("[R%08x", mxarray_varid(tuple_get(out, 0)));
    for(i = 1; i < outargs; i++)
      cmd = sprint("%s, R%08x", cmd, mxarray_varid(tuple_get(out, i)));
    if(args != none_obj) {
      cmd = sprint("%s] = %s(", cmd, fn);
      for(i = 0; i < tuple_length(args) - 1; i++)
	cmd = sprint("%sR%08x, ", cmd, mxarray_varid(tuple_get(args, i)));
      cmd = sprint("%sR%08x);", cmd, mxarray_varid(tuple_get(args, i)));
    }
    else {
      cmd = sprint("%s] = %s;", cmd, fn);
    }
    matlab_evalstring(cmd);
    return out;
  }
}


/************************************************************
 *
 * static helper functions
 *
 ************************************************************/


char *varid_asstring(char *buffer, varid_t v)
{
  static char *digits = "0123456789abcdef";

  for(int i = 7; i >= 0; i--) {
    buffer[i] = digits[v & 0xf];
    v >>= 4;
  }
  return buffer + 8;
}

/************************************************************
 *
 * virtual table functions
 *
 ************************************************************/

static 
object_t matlab_vt_lookup(object_t o, symbol_tr s)
{
  CHECK(o);
  matlab_tr m = o;

  object_t retval = object_lookup(o, s);
  if(!retval) {
#ifdef MATLAB_R13
    static char *undef = ">>\n??? Undefined";
#elif MATLAB_R12
    static char *undef = "??? Undefined";
#else
#error "Unknown MATLAB Version"
#endif
    
    start_matlab_if_necessary();

    engEvalString(m->eng, symbol_name(s));

    // if there is output which does not begin with "??? undefined" ->
    // we have found the symbol
    if(strncmp(m->outputbuffer, undef, strlen(undef)) != 0) {
      return mxfunc_new(symbol_name(s));
    }
    else {
      if(object_parent(o))
	retval = rha_lookup(object_parent(o), s);
    }
  }

  return retval;
}

#undef CHECK
