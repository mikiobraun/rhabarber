#include "extensions.h"

#include <stdio.h>
#include "symbol_tr.h"
#include "object.h"
#include "builtin_tr.h"
#include "list_tr.h"
#include "symtable.h"
#include "eval.h"

/***********************************************************************
 * Language additions
 */ 

symbol_tr disp_sym = 0;
symbol_tr slots_sym = 0;
symbol_tr snatch_sym = 0;
symbol_tr match_sym = 0;

BUILTIN(b_disp);
BUILTIN(b_slots);
BUILTIN(b_snatch);
BUILTIN(b_match);

void utils_init(object_t root)
{
  // check initialization order
  assert(builtin_type);

  // symbols for "utils"
  disp_sym = symbol_new("disp");
  slots_sym = symbol_new("slots");
  snatch_sym = symbol_new("snatch");
  match_sym = symbol_new("match");

  // add utils functions to the object hierarchy
  object_assign(root, disp_sym, builtin_new(&b_disp));
  object_assign(root, slots_sym, builtin_new(&b_slots));
  object_assign(root, snatch_sym, builtin_new(&b_snatch));
  object_assign(root, match_sym, builtin_new(&b_match));
}
