#include "minunit.h"
#include "tuple_fn.h"
#include "symbol_fn.h"
#include "object.h"

symbol_t parent_sym;

/* construct a tuple, check the size, put some things into the tuple and retrieve them later */
BEGIN_CASE(tuple)
     parent_sym = symbol_new("parent");

     tuple_t t = tuple_new(3);
     mu_assert_equal("length of tuple should be 3!", tuple_len(t), 3);
     symbol_t foo = symbol_new("foo"); mu_assert_equal("foo should be 2", foo, 2);
     symbol_t bar = symbol_new("bar"); mu_assert_equal("bar should be 3", bar, 3);
     symbol_t baz = symbol_new("baz"); mu_assert_equal("baz should be 4", baz, 4);
     
     tuple_set(t, 0, wrap(SYMBOL_T, NULL, &foo)); mu_assert_equal("tuple position 0 should be foo (i.e. 2)", *RAW(symbol_t, tuple_get(t, 0)), 2);
     tuple_set(t, 1, wrap(SYMBOL_T, NULL, &bar)); mu_assert_equal("tuple position 1 should be bar (i.e. 3)", *RAW(symbol_t, tuple_get(t, 1)), 3);
     tuple_set(t, 2, wrap(SYMBOL_T, NULL, &baz)); mu_assert_equal("tuple position 2 should be baz (i.e. 4)", *RAW(symbol_t, tuple_get(t, 2)), 4);
END_CASE

BEGIN_TESTS
mu_run_test(tuple);
END_TESTS
