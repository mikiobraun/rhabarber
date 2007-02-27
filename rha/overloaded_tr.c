#include "overloaded_tr.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

//#define DEBUG
#include "debug.h"

#include "fmt.h"
#include "messages.h"

#include "core.h"
#include "eval.h"
#include "object.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"
#include "list_tr.h"
#include "function_tr.h"
#include "plain_tr.h"
#include "treeobjectobject.h"


// NOTE: typical unary overloaded fn are:  'from' (converters)
//       typical binary overloaded fn are: 'plus' (operators)

struct overloaded_s
{
  RHA_OBJECT;
  treeobjectobject_t impl; // registered functions
  treeobjectobject_t look; // derived functions
  bool dirty;              // dirty flag, recompute if set
  object_t this;
  // no 'priority', overloaded should not be used as a prule
  // overloaded itself can not be macros, however, the assigned function/builtins can
};

#define CHECK(x) CHECK_TYPE(overloaded, x)

primtype_t overloaded_type = 0;
struct rhavt overloaded_vt;

//////////////////////////////////////////////////////////////////////
// Forward declarations
static void fmt_tuplenames(STREAM *s, char *spec, va_list *ap);
static object_t resolve(overloaded_tr ov, tuple_tr t);
static void recompute(overloaded_tr ov);
static tuple_tr promotekey(tuple_tr t, int i, object_t cv);
static object_t promotefct(object_t f, int i, object_t cv);
static list_tr getconverters(object_t o);
static tuple_tr computekeys(tuple_tr in);

static object_t overloaded_vt_call(object_t env, tuple_tr in);
static void overloaded_vt_bind(object_t env, object_t this);

static object_t overloaded_resolve_failed_hook
                (overloaded_tr ov, object_t env, tuple_tr t);


//////////////////////////////////////////////////////////////////////
//
// Exported functions
//

/*
 * initialize everything for the overloaded type
 */
void overloaded_init(void)
{
  if (!overloaded_type) {

    overloaded_type  = primtype_new("overloaded", sizeof(struct overloaded_s));
    overloaded_vt = default_vt;
    overloaded_vt.call = overloaded_vt_call;
    overloaded_vt.bind = overloaded_vt_bind;

    primtype_setvt(overloaded_type, &overloaded_vt);

    (void) overloaded_new();

    // special format type for printing signatures
    addfmt('T', fmt_tuplenames);
  }
}


/*
 * create new overloaded object
 */
overloaded_tr overloaded_new()
{
  overloaded_tr ov = object_new(overloaded_type);
  treeobjectobject_init(&ov->impl);
  treeobjectobject_init(&ov->look);
  ov->dirty = false;
  ov->this = 0;
  return ov;
}


/*
 * add a new function for given signature
 *
 * This function also sets the dirty bit such that subsequent calls to
 * resolve trigger the recomputation of the derived functions.
 */
void overloaded_add(overloaded_tr ov, object_t f, tuple_tr signature)
{
  treeobjectobject_insert(&ov->impl, signature, f);
  ov->dirty = true;

  printdebug("Added fn %o for type %T.\n", _O ov, _O signature);
}


/*
 * call an overloaded function
 *
 * first resolve the overloaded function and then just call it
 */
object_t overloaded_vt_call(object_t env, tuple_tr in)
{
  assert(HAS_TYPE(tuple, in));
  assert(tuple_length(in)>0);
  overloaded_tr ov = tuple_get(in, 0);
  assert(HAS_TYPE(overloaded, ov));

  int tlen = tuple_length(in);
  if (tlen >= 2) {

    // evaluate types arguments
    tuple_tr t = computekeys(in);

    // the type of x and the other args are their parents
    object_t fn = resolve(ov, t);
    if (!fn) {
      // call resolve failed hook
      in = overloaded_resolve_failed_hook(ov, env, in);
      t = computekeys(in);

      fn = resolve(ov, t);
    }

    if (!fn) {
      rha_error("Cannot resolve call to overloaded fn %o for type %T.\n", 
		_O ov, _O t);
      return 0;
    } 
    // set the 'this' pointer if existing
    if (ov->this) {
      rha_bind(fn, ov->this);
      ov->this = 0;
    }

    // next time in 'eval': the resolved function will be called
    tuple_set(in, 0, fn);

    // call the resolved function
    return rha_call(env, in);
  }
  else {
    rha_error("Operator %o called with too few arguments.\n", _O ov);
    return 0;
  }
}


static 
void overloaded_vt_bind(object_t ov, object_t this)
{
  CHECK(ov);
  ((overloaded_tr)ov)->this = this;
}

//////////////////////////////////////////////////////////////////////
//
// Local functions
//

/*
 * Resolve a call given a tuple of types
 */
static
object_t resolve(overloaded_tr ov, tuple_tr t)
{
  if (ov->dirty) recompute(ov);

  printdebug("resolving overloaded fn %o for type %T.\n", _O ov, _O t); 
  
  object_t f = treeobjectobject_search(&ov->look, t);

  printdebug("%o\n", _O f);

  return f;  
}


/*
 * Recompute the table of derived functions
 *
 * This function works on a list l which contains the functions are
 * candidates for promotion. Whenever a function gets promoted, it is
 * added to the end of the list. When the list is empty, the algorithm
 * terminates.
 *
 * The function mainly consists of three loops
 * 
 *     initialize l to the actually implemented versions
 *     while l is not empty
 *        for all arguments
 *            for all conversions to the given type
 *                add the promoted function if it does not already exist.
 *
 * I hope this is helpful :)
 *
 * - Mikio
 */
static
void recompute(overloaded_tr ov)
{
  list_tr l = list_from_tree(ov->impl); // list of functions to be promoted

  // first copy the list into to lookup tree
  treeobjectobject_clear(&ov->look);
  tuple_tr data;
  list_foreach(data, l)
    treeobjectobject_insert(&ov->look, tuple_get(data, 0), tuple_get(data, 1));

  // while there are still functions to be promoted
  while (!list_isempty(l)) {
    object_t data = list_pop(l);
    object_t key  = tuple_get(data, 0);
    int      klen = tuple_length(key);

    // for each argument
    for (int i = 0; i < klen; i++) {
      object_t o = tuple_get(key, i);

      // look for converters that convert to o
      list_tr cv_l = getconverters(o);
      if (!cv_l) continue;

      while (!list_isempty(cv_l)) {
	object_t cv_data = list_pop(cv_l);
	object_t cv_key = tuple_get(cv_data, 0);

	// create the promoted key and check if it already exists
	tuple_tr pr_key = promotekey(key, i, cv_key);
	if ( treeobjectobject_search(&ov->look, pr_key) )
	  continue;
	
	// if it doesn't, add the promoted function to the lookup
	// table and to l
	printdebug("added fn %o for type %T.\n", ov, pr_key);
	
	object_t pr_fn =  promotefct(tuple_get(data, 1), 
				     i, 
				     tuple_get(cv_data, 1));
	
	treeobjectobject_insert(&ov->look, pr_key, pr_fn);
	
	list_append(l, tuple_make2(pr_key, pr_fn));
      } /* while cv_list is not empty */
    } /* for all arguments */
  } /* while there are functions to convert */

  ov->dirty = false;
}


/*
 * get converters to a given object
 *
 * the converters to o are given as the overloaded function o.from .
 * All of these converters must take exactly one argument, which is
 * also checked here.
 *
 * If there are no converters or the o.from is not an overloaded
 * function, 0 is returned. Otherwise a list of pairs (key, fct) of
 * the available converters.
 */
static
list_tr getconverters(object_t o) 
{
  // look for o.from 
  overloaded_tr cv = rha_lookup(o, from_sym);
  if (!cv) 
    return 0;
  if (!HAS_TYPE(overloaded, cv)) {
    rha_warning("converter of %o is not an overloaded functions.\n", _O o);
    return 0;
  }

  // convert to list
  list_tr cv_l = list_from_tree(cv->impl);

  // check number of args for the whole list
  object_t x;
  list_foreach(x, cv_l)
    if ( tuple_length(tuple_get(x, 0)) != 1 )
      rha_warning("converter of %o does not have exactly one arg.\n", _O o);

  return cv_l;
}


/*
 * Compute keys
 *
 * Given a call tuple, return a tuple which contains the types
 */
static
tuple_tr computekeys(tuple_tr in)
{
  int tlen = tuple_length(in);
  tuple_tr t = tuple_new(tlen - 1);

  for (int i = 1; i<tlen; i++) {
    object_t x = tuple_get(in, i);
    assert(x);
    tuple_set(t, i-1, object_lookup(x, type_sym));
  }

  return t;
}


/*
 * Promote a single argument in a key (signature)
 *
 * Returns a key where the type of argument i has been replaced
 * by newkey
 */
static
tuple_tr promotekey(tuple_tr key, int i, object_t newkey)
{
  int klen = tuple_length(key);
  tuple_tr pr_key = tuple_new(klen);
  for (int j = 0; j < klen; j++) 
    tuple_set(pr_key, j, tuple_get(key, j));
  tuple_set(pr_key, i, tuple_get(newkey, 0));
  return pr_key;
}


/*
 * Promote a single argument in a function.
 *
 * Construct a function which first calls cv to convert argument i
 * then calls f.
 */
static
object_t promotefct(object_t f, int i, object_t cv)
{
  // create environment
  object_t env;
  if (HAS_TYPE(function, f)) env = function_env(f);
  else env = plain_new();

  // construct argument tuple
  symbol_tr argsyms[2] = { symbol_new("x"), symbol_new("y") };
  tuple_tr args = tuple_make(3, symbol_new("dummy"), argsyms[0], argsyms[1]);

  // construct the code
  tuple_tr cv_code = tuple_make(2, cv, argsyms[i]);
  tuple_tr code = tuple_make(3, f, argsyms[0], argsyms[1]);
  tuple_set(code, i+1, cv_code);

  // create the new function
  function_tr pr_f = function_new(args, 2, code, env);
  return pr_f;
}



//////////////////////////////////////////////////////////////////////
//
// Auxillary functions for printing
//

static
void fmt_tuplenames(STREAM *out, char *spec, va_list *ap)
{
  object_t o = va_arg(*ap, object_t);
  if (tuple_length(o) == 0)
    strputs(out, "[empty]");
  for(int i = 0; i < tuple_length(o); i++) {
    strputs(out, (char*)object_typename(tuple_get(o, i)));
    if (i < tuple_length(o) - 1) strputs(out, " * ");
  }
}


//////////////////////////////////////////////////////////////////////
//
// Function called when resolution failed
//

static object_t
overloaded_resolve_failed_hook(overloaded_tr ov, object_t env, tuple_tr t)
{
  object_t resolve_failed_hook 
    = object_lookup(ov, symbol_new("resolve_failed_hook"));
  if(resolve_failed_hook) {
    object_t retval = object_callslot(ov, "resolve_failed_hook", 2, env, t);
    return retval;
  }
  else
    return t;
}

#undef CHECK
