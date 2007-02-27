#include "plain_tr.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "messages.h"
#include "primtype.h"
#include "symtable.h"
#include "string_tr.h"
#include "symbol_tr.h"
#include "utils.h"

struct plain_s
{
  RHA_OBJECT;
  /* empty */
};

primtype_t plain_type = 0;

//  no checks because all objects are children of plain
//  #define CHECK(x) CHECK_TYPE(plain, x)

void plain_init(void)
{
  if (!plain_type) {
    plain_type = primtype_new("plain", sizeof(struct plain_s));
    (void) plain_new();
  }
}


plain_tr plain_new()
{
  plain_tr v = object_new(plain_type);
  return v;
}


string_t plain_to_string(plain_tr v)
{
  primtype_t pt = object_primtype(v);
  if (pt) {
    string_t repr = sprint("[%s", primtype_name(pt));
    object_t x; // iter var
    list_foreach(x, symtable_tolist(object_table(v))) {
      string_t slotdescr;
      object_t key  =  tuple_get(x, 0);
      object_t value = tuple_get(x, 1);
      if (HAS_TYPE(plain, value))
	slotdescr = sprint(" %o:[plain ...]", key);
      else
	slotdescr = sprint(" %o:%o", key, value); 
      repr = string_plus_string(repr, slotdescr);
    }

    return string_plus_string(repr, "]");
  }
  else
    return sprint("[obj w/o primtype]");
}


bool plain_less_plain(plain_tr v1, plain_tr v2)
     // plain_less_plain is for hashes of arbitrary objects
{
  rha_warning("Comparing internals to yield a results.\n");
  return (v1<v2);
}

object_t plain_lookup(plain_tr p, symbol_tr t)
{
  return rha_lookup(p, t);
}

//#undef CHECK
