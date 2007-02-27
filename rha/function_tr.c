/*
 * function_tr - functions with code in rhabarber
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

/**background**
 *
 * CONSTRUCTION OF THE ACTIVATION RECORD
 *
 *
 * f(argnames) is defined in senv
 * it is called f(args) in cenv
 * if f is a bound method, its "this" pointer has been set to f
 *
 * The activation record is constructed as follows: 
 *
 *               senv
 *                ^
 *                |
 *                |
 *  this <----- thisproxy  (optional, only if *_bind has been called)
 *        this  
 *                ^
 *                | parent
 *                |
 *                ar.argnames = args
 *                  .recur = f
 *                  .env = cenv
 *
 * Note that the lookup order is "local" - "this" - "parent".
 */

#include "function_tr.h"

#include <assert.h>
#include <stdio.h>
#include "messages.h"
#include "eval.h"
#include "core.h"
#include "object.h"
#include "primtype.h"
#include "none_tr.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "plain_tr.h"
#include "utils.h"
#include "thisproxy_tr.h"

struct function_s
{
  RHA_OBJECT;
  int numargs;   // number of arguments
  tuple_tr args; // names of the arguments
  object_t code; // code of the function
  object_t env;  // syntactic environment
  
  object_t this; // the receiver of the next call

  double priority; // if priority>0 the function is a prule
  bool ismacro;    // if true, the args are not evaluated
};

#define CHECK(f) CHECK_TYPE(function, f)

primtype_t function_type = 0;
struct rhavt function_vt;

static object_t    function_vt_call(object_t env, tuple_tr in);
static void        function_vt_bind(object_t f, object_t this);
static double      function_vt_priority(object_t f);
static bool        function_vt_ismacro(object_t f);

void function_init()
{
  if (!function_type) {
    tuple_init();
    plain_init();

    function_type = primtype_new("function", sizeof(struct function_s));
    function_vt = default_vt;
    function_vt.call = &function_vt_call;
    function_vt.bind = &function_vt_bind;
    function_vt.priority = &function_vt_priority;
    function_vt.ismacro = &function_vt_ismacro;
    primtype_setvt(function_type, &function_vt);

    (void) function_new(tuple_new(1), 
			0, tuple_new(0), plain_new(0));
  }
}


function_tr function_new(tuple_tr t, int numargs, object_t code, object_t env)
{
  int len = tuple_length(t);
  // note that there are two cases:
  // case 1:   numargs==len-2: t contains args and code
  //     this is for (called from eval:eval()
  //              f = fn(x) 2*x
  //     here t is the rhs
  // case 2:   numargs==len-1: t contains only args
  //     this is for (called from eval:assign()
  //              f(x) = 2*x
  //     here t is the lhs
  // this var avoids unnecessary copying elsewhere

  assert(len > numargs);
  // t[0] is "fn" or the name of the function
  // t[1], ..., t[numargs] are the args
  // t[len-1] contains the code if numargs<len-1

  function_tr f = object_new(function_type);
  f->numargs = numargs;
  f->code = code;
  f->env = env;   // remember the lexical scope, which is the
	          // scope when the function appeared lexically
  f->args = tuple_new(numargs);
  for (int i=0; i<numargs; i++) {
    if (!HAS_TYPE(symbol, tuple_get(t, i+1))) {
      rha_error("cannot calldef %o, or non-symbol argument "
		"in function definition.\n", tuple_get(t, 0));
    }
    tuple_set(f->args, i, tuple_get(t, i+1));
  }
  f->priority = 0.0;  // default is that a function is not a prule
  f->ismacro = false;   // default is that a function is not a macro
  return f;
}


function_tr function_new_prule(tuple_tr t, int numargs, object_t code, object_t env, double priority)
{
  function_tr f = function_new(t, numargs, code, env);
  f->priority = priority;
  return f;
}


function_tr function_new_macro(tuple_tr t, int numargs, object_t code, object_t env)
{
  function_tr f = function_new(t, numargs, code, env);
  f->ismacro = true;
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
  assert(0 <= i);
  assert(i < f->numargs);
  return tuple_get(f->args, i);
}

string_t function_argname(function_tr f, int i)
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


object_t function_env(function_tr f)
{
  CHECK(f);
  return f->env;
}


void function_setenv(function_tr f, object_t env)
{
  CHECK(f);
  f->env = env;
}


string_t function_to_string(function_tr f)
{
  CHECK(f);
  string_t s;
  if (f->priority > 0.0)
    s = gc_strdup("[prule with args (");
  else if (f->ismacro)
    s = gc_strdup("[macro with args (");
  else
    s = gc_strdup("[function with args (");
  for(int i = 0; i < f->numargs; i++) {
    if (i) s = string_plus_string(s, " ");
    s = string_plus_string(s, function_argname(f, i));
  }
  s = string_plus_string(s, ") and code ");
  object_t code = f->code;
  if (code) s = string_plus_string(s, object_to_string(code));
  if (f->this) 
    s = string_plus_string(s, sprint(" bound to object %o", f->this));
  s = string_plus_string(s, "]");
  return s;
}

/************************************************************
 *
 * rhavt functions
 *
 ************************************************************/

double function_vt_priority(object_t o)
{
  CHECK(o);
  return ((function_tr)o)->priority;
}


bool function_vt_ismacro(object_t o)
{
  CHECK(o);
  return ((function_tr)o)->ismacro;
}


void function_vt_bind(object_t o, object_t this)
{
  CHECK(o);
  ((function_tr)o)->this = this;
}

/* call a function written in rhabarber
 * 
 * Calls the function with the call tuple t
 * t[0] contains the function itself, and t[1]..t[nargs] are the arguments
 * also passed is "env", the callers environment
 */
object_t function_vt_call(object_t env, tuple_tr t)
{
  assert(HAS_TYPE(tuple, t));
  assert(tuple_length(t)>0);
  function_tr f = tuple_get(t, 0);
  assert(HAS_TYPE(function, f));

  // do the number of parameters of the call match the definition?
  int argc = function_numargs(f);
  if(argc != tuple_length(t) - 1) {
    rha_error(argc == 1 
	      ? "Exactly %d argument expected.\n"
	      : "Exactly %d arguments expected.\n", argc);
    return 0;
  }

  // construct the activation record
  object_t ar;
  if (f->this) {
    object_t tp = thisproxy_new(f->this);
    ar = plain_new();
    object_setparent(ar, tp);
    object_setparent(tp, f->env);
    printdebug("Bound to %x:%o\n", f->this, f->this);
    f->this = 0;
  }
  else
    ar = object_clone(f->env);

  // call fctcall hook
  t = eval_fctcall_hook(env, t);

  // assign args to the activation record
  for(int i = 0; i < argc; i++) {
    object_t o =  tuple_get(f->args, i);
    assert(HAS_TYPE(symbol, o));    // for now without constraints
    // note that the scope of the args is only env, not "this" as well
    object_t arg = tuple_get(t, i+1);
    assert(arg);
    object_assign(ar, o, arg);
  }
  object_assign(ar, symbol_new("recur"), f);  // for anonymous recursion
  object_assign(ar, env_sym, env);   // to call eval

  if (f->code) {
    // execute the code of the par
    object_t res = none_obj;
    begin_frame(FUNCTION_FRAME)
      res = eval(ar, f->code);
    end_frame(res);
    return res;
  }
  rha_error("function has no code.\n");
  return 0;
}


