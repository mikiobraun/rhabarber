#include "function_tr.h"

#include <assert.h>
#include "core.h"
#include "eval.h"

struct function_s
{
  RHA_OBJECT;
  int numargs;
  tuple_tr args;
  object_t code;
  object_t code_env;
  // object_t constraints;
};

primtype_t function_type = 0;
object_t function_obj = 0;

#define CHECK(f) CHECK_TYPE(function, f)

function_tr function_new(tuple_tr t, int numargs, object_t code, object_t env)
{
  int len = tuple_length(t);
  // note that for
  //    numargs==len-2: t contains args and code
  //    numargs==len-1: t contains only args
  // this var avoids unnecessary copying elsewhere

  assert(len > numargs);
  // t[0] is "fn" or the name of the function
  // t[1], ..., t[numargs] are the args
  // t[len-1] contains the code if numargs<len-1

  function_tr f = object_new(function_type);
  OBJECT_INIT(f, function);
  f->numargs = numargs;
  f->code = code;
  f->code_env = env;
  if (len==numargs+2) tuple_set(t, len-1, 0);  // get rid of the code in t
  f->args = t;
  return f;
}

int function_numargs(function_tr f)
{
  CHECK(f);
  return f->numargs;
}

object_t function_arg(function_tr f, int i)
{
  CHECK(f);
  assert(0 < i);
  assert(i < f->numargs);
  return tuple_get(f->args, i);
}

const char *function_argname(function_tr f, int i)
{
  CHECK(f);
  return symbol_name(tuple_get(f->args, i));
}

object_t function_code(function_tr f)
{
  CHECK(f);
  return f->code;
}

void function_setcode(function_tr f, object_t code)
{
  CHECK(f);
  f->code = code;
}

int check_assign(object_t expr)
     // 5             returns 0  == the otherwise case
     // x=5           returns 1
     // f(x)=5        returns 1
     // g(x).f(y)=4   returns 2
{
  if (!HAS_TYPE(tuple, expr)) return 0;
  if (tuple_length(expr)!=3) return 0;
  object_t t0 = tuple_get(expr, 0);
  if (!HAS_TYPE(symbol, t0)) return 0;
  if (!symbol_equal_symbol(t0, assign_sym)) return 0;
  object_t t1 = tuple_get(expr, 1);
  if (HAS_TYPE(symbol, t1)) return 1;    // e.g. x=5
  if (!HAS_TYPE(tuple, t1)) return 0;
  if (!tuple_length(t1)>0) return 0;
  object_t t10 = tuple_get(t1, 0);
  if (!HAS_TYPE(symbol, t10)) return 0;
  if (is_keyword(t10)) return 2;   // e.g. g(x).f(y) = 4
  return 1;
}

object_t input_getsymbol(object_t expr)
{
  assert(check_assign(expr)==1);
  return tuple_get(expr, 1);
}

object_t function_call(object_t par, object_t this, object_t env, tuple_tr in)
     // par == prototype activation record
     // this == the message receiver, this is not always parent.par
     // aar == activated activation record
{
  CHECK(par);
  CHECK_TYPE(tuple, in);
  function_tr f = par;

  // do the number of parameters of the call match the definition
  int numargs = function_numargs(f);
  int numinputs = tuple_length(in) - 1;
  if(numargs < numinputs) return 0;

  // parent of aar is par, this allows templates to find other members
  object_t aar = object_clone(par);

  // if "par" has a non-zero "that" pointer, "par" is member function
  // in that case set "that" pointer to message receiver which is "this"
  if (object_that(par)) object_setthat(aar, this);

  // do the argument stuff
  // FIRST evaluate the default values and collect the symbols
  object_t arg;
  for (int i = 1; i <= numargs; i++) {
    arg = function_arg(f, i);
    assert(arg_check(arg));
    symbol_tr s = arg_getsymbol(arg);
    assert(s);
    object_t defaultcode = arg_getdefault(arg);
    if (defaultcode) {
      object_t obj = eval(aar, env, defaultcode);
      if (!obj) return 0;
      object_assign(aar, s, obj);
    }
  }
  // SECOND process the args without assignment
  int i = 1;
  while (i <= numinputs) {
    int ca = check_assign(arg = tuple_get(in, i));
    if (ca == 0) break;
    if (ca == 2) return 0;
    object_t obj = eval(this, env, arg);
    if (!obj) return 0;
    object_assign(aar, arg_getsymbol(function_arg(f, i)), obj);
    i++;
  }
  // THIRD process the out of order stuff
  while (i <= numinputs) {
    int ca = check_assign(arg = tuple_get(in, i));
    if (ca != 1) return 0;
    object_t s = tuple_get(arg, 1);
    assert(s);
    object_t obj = eval(this, env, tuple_get(arg, 2));
    if (!obj) return 0;
    object_assign(aar, s, obj);
    i++;
  }


  if (f->code == 0) {
    // this is a template (or a virtual function)
    return aar;
  }
  else {
    // execute the code of the par
    return eval(aar, f->code_env, f->code); 
  }
}

