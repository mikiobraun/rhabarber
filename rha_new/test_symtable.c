#include "minunit.h"
#include "symtable.h"
#include "object.h"
#include "symbol_fn.h"

symbol_t parent_sym;

BEGIN_CASE(symtable)
     parent_sym = symbol_new("parent");

     symtable_t s = symtable_new();

     symbol_t xsym = symbol_new("x");
     object_t x = wrap(SYMBOL_T, NULL, &xsym);
     
     mu_assert_equal_ptr("trying to look up non-existent symbol:\n", symtable_lookup(s, xsym), NULL);

     symtable_assign(s, xsym, x);

     mu_assert_equal_ptr("trying to look up 'x' in symtable:\n", symtable_lookup(s, xsym), x);
END_CASE

BEGIN_TESTS
  mu_run_test(symtable);
END_TESTS
