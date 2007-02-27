#include "arithmetics.h"

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdarg.h>
#include "builtin_tr.h"
#include "core.h"
#include "eval.h"
#include "function_tr.h"
#include "void_tr.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "usage.h"
#include "tictoc.h"

//#define DEBUG
#include "debug.h"

#include "treeobjectpairobject.h"
#include "treesymboltopo.h"
#include "treeobjecttoo.h"

/*
 * overloading stuff
 */

/* 
 * arithmetics engine state
 */
static object_t storedroot;          // environment into which the op's are inserted
static bool dirtyflag;               // have the tables been altered?

// 'optable' contains all implemented versions:
// topo == tree: object pair -> object
// maps as follows: symbol -> (type * type -> fct)
static treesymboltopo_t optable;     
static treesymboltopo_t lookuptable; // contains all possible versions: used for lookup in arithmetics_resolve

// 'convtable' contains the converters
// too  == tree: goal:object -> dest:object
// maps as follows: dst -> (src -> fct)
static treeobjecttoo_t convtable;    

/*
 * local functions
 */
static void inittable();
static void addoperator(treesymboltopo_t *table, symbol_tr s, object_t fct, object_t t1, object_t t2);
static void addconvert(object_t fct, object_t from, object_t to);
static object_t promotefct(object_t fct, int arg, object_t convfct);
static bool recomputetable();
static void dumptables();
static object_t resolve(symbol_tr s, object_t a1, object_t a2);

BUILTIN(call_op);


/**********************************************************************

Interface

**********************************************************************/

void arithmetics_stub_init(object_t root)
{
  // requires
  symbol_init();
  list_init();
  builtin_init();

  // add overloading to rhabarber
  inittable();
  storedroot = root;
  dirtyflag = false;
}


void arithmetics_overload(symbol_tr op, 
			  object_t fct, 
			  object_t typ1, object_t typ2)
{
  printdebug("arithmetics_overload( %s : %s * %s )\n", symbol_name(op),
	     object_name(typ1), object_name(typ2));
  addoperator(&optable, op, fct, typ1, typ2);
}


void arithmetics_convert(object_t fct, object_t typ1, object_t typ2)
{
  printdebug("arithmetics_convert( %s -> %s )\n", 
	  object_name(typ1), object_name(typ2));
  addconvert(fct, typ1, typ2);
}

void arithmetics_recomputetable()
{
  tic();
  recomputetable();
  printf("Rebuild overload table in %fs.\n", toc());
#ifdef DEBUG
  dumptables();
#endif
}

object_t arithmetics_resolve(symbol_tr op, object_t arg1, object_t arg2)
{
  return resolve(op, object_parent(arg1), object_parent(arg2));
}


void arithmetics_dumptables()
{
  printdebug("arith: table dump\n");
  dumptables();
}

/**********************************************************************

Implementation

**********************************************************************/

/*
 * Initialize the tables
 */

static void inittable()
{
  treesymboltopo_init(&optable);
  treeobjecttoo_init(&convtable);
  treesymboltopo_init(&lookuptable);
}


/*
 * Add an operator.
 *
 * Add fct under s->(t1,t2). Create new list for symbol s if necessary.
 */
static void addoperator(treesymboltopo_t *table, symbol_tr s, object_t fct, object_t t1, object_t t2)
{
  topo_t t = treesymboltopo_search(table, s);
  if (!t) {
    NEW(t);
    treeobjectpairobject_init(t);
    treesymboltopo_insert(table, s, t);
  }

  objectpair_t op;
  NEW(op);
  op->first = t1;
  op->second = t2;

  treeobjectpairobject_insert(t, op, fct);
  dirtyflag = true;
}


/*
 * Add a converter
 */
static void addconvert(object_t fct, object_t from, object_t to)
{
  too_t t = treeobjecttoo_search(&convtable, to);
  if (!t) {
    NEW(t);
    treeobjectobject_init(t);
    treeobjecttoo_insert(&convtable, to, t);
  }

  treeobjectobject_insert(t, from, fct);
  dirtyflag = true;
}


/*
 * Recompute all tables
 *
 * This is the only slightly involved function. Basically, we use the
 * convtable to extend the definitions in the optable.
 */
#define TREE_FOREACH(tree, t, it) \
for(tree##_begin(&it, t); !tree##_done(&it); tree##_next(&it)) 

static bool recomputetable()
{
  builtin_tr b = builtin_new(call_op);

  int defined = 0;
  int inferred = 0;

  // a big loop over all symbols
  treesymboltopo_iterator_t si;
  TREE_FOREACH(treesymboltopo, &optable, si) {
    symbol_tr s = treesymboltopo_get_key(&si);
    topo_t t = treesymboltopo_get_value(&si);

//--> copyoperators(treesymboltopo_t, treesymboltopo_t);
    // initialize table with actually defined functions
    treeobjectpairobject_iterator_t pi;
    TREE_FOREACH(treeobjectpairobject, t, pi) {
      objectpair_t op = treeobjectpairobject_get_key(&pi);
      object_t fct = treeobjectpairobject_get_value(&pi);
      
      printdebug("Adding %s : %s * %s\n", symbol_name(s), object_name(op->first), object_name(op->second));
      addoperator(&lookuptable, s, fct, op->first, op->second);
      object_assign(storedroot, s, b);
      defined++;
    }
//--> end of copyoperators
    // now: iterate until nothing changes: add converted functions
    /* 
     * implementation note:
     *
     * we're doing it here rather brute force, but frankly speaking,
     * the code is quite horrible as it is and organizing it such that
     * the complete tree is generated for each symbol first does not
     * seem to lead to cleaner code.
     */
    treeobjectpairobject_t *defops = treesymboltopo_search(&lookuptable, s);

    bool changed;
    do {      
      changed = false;
//-> search for first operator which can be promoted and has not been promoted yet
      TREE_FOREACH(treeobjectpairobject, defops, pi) {
	objectpair_t op = treeobjectpairobject_get_key(&pi);
	object_t fct = treeobjectpairobject_get_value(&pi);

	// first arg
	printdebug("Searching for converters to %s\n", object_name(op->first));
	too_t ot = treeobjecttoo_search(&convtable, op->first);
	if (ot) {
	  treeobjectobject_iterator_t oi;
	  TREE_FOREACH(treeobjectobject, ot, oi) {
	    object_t from = treeobjectobject_get_key(&oi);
	    object_t convfct = treeobjectobject_get_value(&oi);

	    struct objectpair op2 = { from, op->second };
	    // did we already define the combination op2?
	    if (!treeobjectpairobject_search(defops, &op2)) {
//-> okay, let's promote it and add
	      printdebug(" Adding %s : %s * %s\n", symbol_name(s), object_name(from), object_name(op->second));
#ifdef debug
	      object_print(promotefct(fct, 1, convfct));
#endif
	      addoperator(&lookuptable, s, promotefct(fct, 1, convfct), from, op->second);
	      changed = true;
	      inferred++;
	      break;
	    }
	  }
	}
	else printdebug("  none found\n");

//-> and up to the next round!
	// if something has changed, we have to leave immediatly,
	// because the iterator is no longer valid.
	if (changed) break; // exit to outermost do-while loop

	// second arg
	printdebug("Searching for converters to %s\n", object_name(op->second));
	ot = treeobjecttoo_search(&convtable, op->second);
	if (ot) {
	  treeobjectobject_iterator_t oi;
	  TREE_FOREACH(treeobjectobject, ot, oi) {
	    object_t from = treeobjectobject_get_key(&oi);
	    object_t convfct = treeobjectobject_get_value(&oi);

	    struct objectpair op2 = { op->first, from };

	    if (!treeobjectpairobject_search(defops, &op2)) {
	      printdebug(" Adding %s : %s * %s\n", symbol_name(s), object_name(op->first), object_name(from));
	      addoperator(&lookuptable, s, promotefct(fct, 2, convfct), op->first, from);
	      changed = true;
	      inferred++;
	      break;
	    }
	  }
	}
	else printdebug("  none found\n");

	// if something has changed, we have to leave immediatly,
	// because the iterator is no longer valid.
	if (changed) break; // exit to outermost do-while loop
      }
    } while(changed);
    printdebug("--end of %s\n", symbol_name(s));
  }
  
  dirtyflag = false;
  
  fprintf(stderr,
	  "arithmetics overloading table has %d functions,\n"
	  "   %d defined and\n"
	  "   %d inferred.\n",
	  defined + inferred, defined, inferred);
  
  return 0;
}


/*
 * Promote a single argument in a function.
 *
 * Construct an environment which contains "convert" and "fct" and
 * construct a new function in this environment which first promotes
 * an argument using convert and then calls fct.
 */
object_t promotefct(object_t fct, int arg, object_t convfct)
{
  // construct the environment of fct
  object_t env = object_new(void_type);

  // construct argument tuple
  symbol_tr argsyms[3] = { 0, symbol_new("x"), symbol_new("y") };
  tuple_tr args = tuple_make(4, symbol_new("dummy"), argsyms[1], argsyms[2], 0);

  // construct the code
  tuple_tr convcode = tuple_make(3, convfct, argsyms[arg], 0);

  tuple_tr code = tuple_make(4, fct, argsyms[1], argsyms[2], 0);
  tuple_set(code, arg, convcode);

  function_tr f = function_new(args, 2, code, env);
  //  object_print(f);
  return f;
}


/*
 * Resolve a call to an operator
 *
 * Simply look up s->(a1, a2) and return the function.
 */
static object_t resolve(symbol_tr s, object_t a1, object_t a2)
{
  if(dirtyflag) recomputetable();

  printdebug("resolving %s, for %s * %s -> ", symbol_name(s), object_name(a1), object_name(a2));

  topo_t t = treesymboltopo_search(&lookuptable, s);
  if (!t) {
    // Hm, this symbol is not in our list of resolved objects!
    fprintf(stderr, "Symbol %s is not overloaded!\n", symbol_name(s));
    return 0;
  }

  struct objectpair op = { a1, a2 };
  object_t f = treeobjectpairobject_search(t, &op);
#ifdef DEBUG
  object_print(f);
#endif
  printdebug("\n");

  return f;  
}


void dumptables()
{
  treesymboltopo_iterator_t ti;
  TREE_FOREACH(treesymboltopo, &lookuptable, ti) {
    treeobjectpairobject_iterator_t topi;
    TREE_FOREACH(treeobjectpairobject, treesymboltopo_get_value(&ti), topi) {
#ifdef DEBUG
      printdebug("%s : %s * %s -> ",
		 symbol_name(treesymboltopo_get_key(&ti)),
		 object_name(treeobjectpairobject_get_key(&topi)->first),
		 object_name(treeobjectpairobject_get_key(&topi)->second));
      object_print(treeobjectpairobject_get_value(&topi));
      printdebug("\n");
#endif
    }
  }
}

/**********************************************************************

Lanugage specific stuff

**********************************************************************/

/*
 * builtin functions
 */

BUILTIN(call_op) {
  assert(tuple_length(in) == 3);
  symbol_tr s = tuple_get(in, 0);
  CHECK_TYPE(symbol, s);
  object_t x = eval(env, tuple_get(in, 1));
  if (!x) return 0;
  object_t y = eval(env, tuple_get(in, 2));
  if (!y) return 0;
  object_t fn = arithmetics_resolve(s, x, y);
  if (!fn) {
    fprintf(stderr, "Cannot resolve call to operator %s with types %s, %s\n", symbol_name(s),
	    object_name(x), object_name(y));
    return 0;
  } 
  tuple_set(in, 0, fn);
  tuple_set(in, 1, x);
  tuple_set(in, 2, y);
  return eval(0, in);
}


/**********************************************************************

Auxillaries

**********************************************************************/

/*
static const char* getname(object_t x)
{
  object_t n = object_lookup(x, symbol_new("name"));
  if(!n) {
    static char ptr[32];
    sprintf(ptr, "%p", x);
    n = string_new(ptr);
  }
  if (HAS_TYPE(string, n)) {
    return string_get(n);
  }
  return 0;
}
*/
