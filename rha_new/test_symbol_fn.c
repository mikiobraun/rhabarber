#include "minunit.h"
#include "symbol_fn.h"

BEGIN_CASE(symbols)
  mu_assert_equal("first symbol not 1!", symbol_new("hello!"), 1);
  mu_assert_equal("second symbol not 2!", symbol_new("yessss!"), 2);
  mu_assert_equal("third symbol should be 3!", symbol_new("well, well"), 3);
  mu_assert_equal("look up first symbol again failed!", 
	    symbol_new("hello!"), 1);
END_CASE

BEGIN_TESTS
mu_run_test(symbols);
END_TESTS
