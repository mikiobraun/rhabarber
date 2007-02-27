/*
 * dict - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#include "dict_tr.h"

#include "object.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "eval.h"
#include "none_tr.h"
#include "bool_tr.h"
#include "messages.h"
#include "cstream.h"
#include "utils.h"
#include "core.h"

// dict uses the new intptr_t-generic containers
#include "gtree.h"



struct dict_s
{
  RHA_OBJECT;
  gtree_t gtree;
};

#define CHECK(x) CHECK_TYPE(dict, x)

primtype_t dict_type = 0;
struct rhavt dict_vt;
/* forward declarations */
static bool dict_lessfct(intptr_t a, intptr_t b);
static bool dict_vt_callable(object_t);
static object_t dict_vt_call(object_t, tuple_tr);
static object_t dict_vt_calldef(object_t, tuple_tr, object_t);

void dict_init(void)
{
  if (!dict_type) {
    dict_type = primtype_new("dict", sizeof(struct dict_s));
    dict_vt = default_vt;
    dict_vt.call = dict_vt_call;
    dict_vt.callable = dict_vt_callable;
    dict_vt.calldef = dict_vt_calldef;
    primtype_setvt(dict_type, &dict_vt);

    /* add more initialization here */
    (void) dict_new();
  }
}

dict_tr dict_new()
{
  dict_tr newdict = object_new(dict_type);
  gtree_init(&newdict->gtree, dict_lessfct);
  return newdict;
}


void dict_insert(dict_tr d, object_t key, object_t val)
{
  gtree_insert(&d->gtree, key, val);
}

object_t dict_search(dict_tr d, object_t key)
{
  object_t o = gtree_searchp(&d->gtree, key);
  return o ? o : none_obj; 
}

int dict_length(dict_tr d)
{
  CHECK(d);
  return gtree_size(&d->gtree);
}


bool dict_lessfct(intptr_t a, intptr_t b)
{
  tuple_tr less 
    = tuple_make3(rha_lookup(root, less_sym), (object_t)a, (object_t)b);
  object_t retval = rha_call(root, less);
  if (!HAS_TYPE(bool, retval)) {
    rha_error("comparison should return boolean value");
  }
  return bool_get(retval);
}


string_t dict_to_string(dict_tr d)
{
  CHECK(d);

  string_t s = "[";
  gtree_iterator_t it;
  gtree_begin(&it, &d->gtree);
  while (!gtree_done(&it)) {
    s = sprint("%s%o:%o", 
	       s, 
	       gtree_get_keyp(&it), 
	       gtree_get_valuep(&it));
    gtree_next(&it);
    if (gtree_done(&it)) break;
    s = sprint("%s, ", s);
  }
  return sprint("%s]", s);
}

/**********************************************************************
 * 
 * Virtual functions
 *
 **********************************************************************/

static bool dict_vt_callable(object_t o)
{
  return true;
}

static object_t dict_vt_call(object_t env, tuple_tr t)
{
  CHECK_TYPE(tuple, t);
  dict_tr d = tuple_get(t, 0);
  CHECK(d);
  int l = tuple_length(t) - 1;
  if (l == 1)
    return dict_search(d, tuple_get(t, 1));
  else {
    tuple_tr key = tuple_new(l);
    for(int i = 0; i < l; i++)
      tuple_set(key, i, tuple_get(t, i + 1));
    return dict_search(d, key);
  }
}

static object_t dict_vt_calldef(object_t o, tuple_tr k, object_t v)
{
  CHECK(o);
  dict_tr d = o;
  if (tuple_length(k) == 1)
    dict_insert(d, tuple_get(k, 0), v);
  else
    dict_insert(d, k, v);
  return o;
}

#undef CHECK
