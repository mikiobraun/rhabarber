/*
 * dict_fn - dict functions
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#include "dict_fn.h"
#include <stdarg.h>
#include "alloc.h"
#include "list_fn.h"
#include "messages.h"
#include "gtree.h"
#include "utils.h"
#include "string_fn.h"

static bool key_lessfct(intptr_t a, intptr_t b)
{
  return a < b;
}

dict_t dict_new(void)
{
  dict_t d = ALLOC_SIZE(sizeof(struct gtree));
  gdict_init(d, key_lessfct);
  return d;
}

int dict_len(dict_t d)
{
  return gtree_size(d);
}

intptr_t dict_hash(any_t obj)
// generates keys
{
  if (ptype(obj)==RHA_int)
    return UNWRAP_INT(obj);
  else
    // for now
    return 42;
}


any_t dict_get(dict_t d, intptr_t key)
{
  return gtree_searchp(d, key);
}

dict_t dict_set(dict_t d, intptr_t key, any_t value)
{
  gtree_insert_(d, key, (intptr_t) value);
  return d;
}

dict_t dict_rm(dict_t d, intptr_t key)
{
  return gtree_deletep(d, key);
}

list_t dict_keys_to_list(dict_t d)
{
  list_t l = list_new();
  gtree_iterator_t *i;
  gtree_begin(i, d);
  while (!gtree_done(i)) {
    list_append(l, gtree_get_key_(i));
  }
  return l;
}

list_t dict_values_to_list(dict_t d)
{
  list_t l = list_new();
  gtree_iterator_t *i;
  gtree_begin(i, d);
  while (!gtree_done(i)) {
    list_append(l, gtree_get_values_(i));
  }
  return l;
}

string_t dict_to_string(dict_t d)
{
  string_t s = sprint("[");
  gtree_iterator_t *i;
  gtree_begin(i, d);
  while (!gtree_done(i)) {
    s = string_concat(s, to_string(gtree_get_key_(i)));
    s = string_concat(s, " => ");
    s = string_concat(s, to_string(gtree_get_value_(i)));
    if (!gtree_done(i))
      s = string_concat(s, ",\n");
  }
  return string_concat(s, "]");
}
