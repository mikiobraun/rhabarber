#include "minunit.h"

bool intless(intptr_t a, intptr_t b)
{
  return a < b;
}

bool strless(intptr_t a, intptr_t b)
{
  return strcmp(a, b) == -1;
}

BEGIN_CASE(gtree)
gtree_t inttree;

gtree_init(&inttree, intless);

mu_assert_equal("looking up non-existent should return 0",
		gtree_searchi(42), 0);
END_CASE

BEGIN_TESTS
mu_run_test(gtree)
END_TESTS
