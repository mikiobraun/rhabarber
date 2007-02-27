/*
 * mxarray - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#include "mxarray_tr.h"


#include "object.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "utils.h"
#include "messages.h"

#include "none_tr.h"
#include "matlab_tr.h"
#include "real_tr.h"
#include "dict_tr.h"

struct mxarray_s
{
  RHA_OBJECT;
  mxArray *array;
  varid_t varid;
};

#define CHECK(x) CHECK_TYPE(mxarray, x)

primtype_t mxarray_type = 0;
object_t mxarray_obj = 0;

/*
 * Forward declarations
 */
static void addprotoslots(mxarray_tr a);

/************************************************************
 *
 * Construction
 *
 ************************************************************/

void mxarray_init(void)
{
  if (!mxarray_type) {
    mxarray_type = primtype_new("mxarray", sizeof(struct mxarray_s));
    /* add more initialization here */
    mxarray_obj = mxarray_new(0, 0);

    // construct pyobject dictionary
    object_assign(mxarray_obj, symbol_new("prototypes"), dict_new());
  }
}

void mxarray_finalizer(GC_PTR obj, GC_PTR cd)
{
  if(HAS_TYPE(mxarray, obj)) { // don't assert, just check
    print("Calling finalizer for %x, %o\n", _O obj, _O obj);
    mxarray_tr a = (mxarray_tr)obj;
    if(a->array) mxDestroyArray(a->array);
    if(a->varid) matlab_clear(a->varid);
  }
}

mxarray_tr mxarray_new(mxArray *a, varid_t vi)
{
  if(a && vi) {
    rha_error("mxarray can only be constructed for either in memory or in matlab");
  }

  mxarray_tr newmxarray = object_new(mxarray_type);
  newmxarray->array = a;
  newmxarray->varid = vi;

  GC_REGISTER_FINALIZER(newmxarray, mxarray_finalizer, NULL, NULL, NULL);

  if(a || vi)
    addprotoslots(newmxarray);

  object_assign(newmxarray, symbol_new("domain"), matlab_domain);

  return newmxarray;
}


/************************************************************
 *
 * Methods
 *
 ************************************************************/

string_t mxarray_to_string(mxarray_tr a)
{
  mxArray *ma = a->array;
  if(!ma) {
    if (a->varid == 0)
      return "mx_NULL";
    else
      return sprint("[mxArray R%08x in Matlab]", a->varid);
  }
  else if(mxIsDouble(ma)) {
    if (mxGetM(ma) == 1 && mxGetN(ma) == 1)
      return sprint("%f", mxGetScalar(ma));
    else if(mxGetM(ma) < 100 && mxGetN(ma) < 10) {
      double *a = mxGetPr(ma);
      int m = mxGetM(ma);
      int n = mxGetN(ma);
      string_t out = gc_strdup("");
      for(int i = 0; i < m; i++) {
	for(int j = 0; j < n; j++)
	  out = sprint("%s%10f   ", out, a[i + m*j]);
	if(i < m-1) 
	  out = sprint("%s\n", out);
      }
      return out;
    }
    else
      return sprint("[%d * %d double mxarray]", mxGetM(ma), mxGetN(ma));
  }
  else if(mxIsChar(ma)) {
    static char buffer[1024];
    mxGetString(ma, buffer, 1024);
    return sprint("%s", buffer);
  }
#ifdef MATLAB_R13
  else if(mxIsLogical)
    return gc_strdup(mxGetLogicals(ma)[0] ? "true" : "false");
#endif
  return sprint("[mxarray at %p]", ma);
}


mxarray_tr mxarray_putobj(mxarray_tr a)
{
  if(a->array) {
    varid_t vi = matlab_newvarid();
    matlab_put(vi, a->array);
    mxDestroyArray(a->array);
    a->array = 0;
    a->varid = vi;
  }
  return a;
}

mxarray_tr mxarray_getobj(mxarray_tr a)
{
  if(a->varid) {
    a->array = matlab_get(a->varid);
    matlab_clear(a->varid);
    a->varid = 0;
    addprotoslots(a);
  }
  return a;
}

mxArray *mxarray_array(mxarray_tr a)
{
  return a->array;
}

varid_t mxarray_varid(mxarray_tr a)
{
  return a->varid;
}

object_t mxarray_get(mxarray_tr a, int i, int j)
{
  mxarray_getobj(a);
  mxArray *ma = a->array;

  if (mxIsDouble(ma)) {
    return real_new(mxGetPr(ma)[i + j * mxGetM(ma)]);
  }
  else {
    rha_error("mxarray_set: mxArray type %s not supported\n", mxarray_typeid(a));
  }
  return 0;
}

object_t mxarray_set(mxarray_tr a, int i, int j, object_t o)
{
  mxarray_getobj(a);
  mxArray *ma = a->array;

  if (!mxIsDouble(ma)) {
    rha_error("mxarray_set: mxArray type %s not supported\n", mxarray_typeid(a));
  }
  if (HAS_TYPE(real, o)) {
    mxGetPr(ma)[i + j * mxGetM(ma)] = real_get(o);
  }
  else if(HAS_TYPE(int, o)) {
    mxGetPr(ma)[i + j * mxGetM(ma)] = int_get(o);
  }
  else {
    rha_error("Cannot store values of type %o in matrix\n",
	      rha_lookup(o, symbol_new("typename")));
  }
  return o;
}

string_t mxarray_varidname(mxarray_tr a)
{
  mxarray_putobj(a);
  return matlab_varidname(mxarray_varid(a));
}

mxarray_tr mxarray_newvar()
{
  return mxarray_new(0, matlab_newvarid());
}

object_t mxarray_binop(string_t op, mxarray_tr arg1, mxarray_tr arg2)
{
  varid_t out = matlab_newvarid();

  mxarray_putobj(arg1);
  mxarray_putobj(arg2);

  matlab_evalstring(sprint("R%08x = R%08x %s R%08x;", 
			   out, 
			   mxarray_varid(arg1), 
			   op, 
			   mxarray_varid(arg2)));
  return mxarray_new(0, out);
}


object_t mxarray_unop(string_t op, mxarray_tr arg)
{
  varid_t out = matlab_newvarid();

  mxarray_putobj(arg);

  matlab_evalstring(sprint("R%08x = %s R%08x;", 
			   out, 
			   op, 
			   mxarray_varid(arg)));
  return mxarray_new(0, out);
}


int mxarray_rows(mxarray_tr a)
{
  mxarray_getobj(a);
  return mxGetM(a->array);
}

int mxarray_cols(mxarray_tr a)
{
  mxarray_getobj(a);
  return mxGetN(a->array);
}

string_t mxarray_typeid(mxarray_tr a)
{
  char *tn = 0;
  mxArray *ma = a->array;

  if(!ma) tn = "remote";
  else {
    if (mxIsLogical(ma)) tn = "logical";
    else if (mxIsChar(ma)) tn = "char";
    else if (mxIsNumeric(ma)) {
      if (mxIsDouble(ma)) tn = "double";
      else if (mxIsInt8(ma)) tn = "int8";
      else if (mxIsInt16(ma)) tn = "int16";
      else if (mxIsInt32(ma)) tn = "int32";
      else if (mxIsInt64(ma)) tn = "int64";
      else if (mxIsUint8(ma)) tn = "uint8";
      else if (mxIsUint16(ma)) tn = "uint16";
      else if (mxIsUint32(ma)) tn = "uint32";
      else if (mxIsUint64(ma)) tn = "uint64";
    }
    else if (mxIsCell(ma)) tn = "cell";
    else if (mxIsStruct(ma)) tn = "struct";
    else tn = "unknown";
  }
  return gc_strdup(tn);
}

/************************************************************
 * scalar values
 */
int mxarray_getint(mxarray_tr a)
{
  mxarray_getobj(a);
  if(mxIsNumeric(a->array))
    return mxGetScalar(a->array);
  else
    rha_error("mxarray is not numeric");
  return 0;
}

double mxarray_getreal(mxarray_tr a)
{
  mxarray_getobj(a);
  if(mxIsNumeric(a->array))
    return mxGetScalar(a->array);
  else
    rha_error("mxarray is not numeric");
  return 0;
}

/************************************************************
 *
 * Static helper functions
 *
 ************************************************************/

static void addprotoslots(mxarray_tr a)
{  
  object_t ptdict = object_lookup(mxarray_obj, symbol_new("prototypes"));
  if (ptdict) {
    object_t prototype = dict_search(ptdict, string_new(mxarray_typeid(a)));
    
    if(prototype != none_obj) {
      object_snatch(a, prototype);
    }
  }
}

#undef CHECK

