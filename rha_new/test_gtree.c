#include "minunit.h"

#include "gtree.h"

bool intless(intptr_t a, intptr_t b)
{
  return a < b;
}

bool strless(intptr_t a, intptr_t b)
{
  return strcmp((void*)a, (void*)b) < 0;
}

BEGIN_CASE(gtree)

   // test the integer tree
   gtree_t inttree;

   gtree_init(&inttree, intless);

   mu_assert_equal("looking up non-existent should return 0",
	           gtree_searchi(&inttree, 42), 0);

   gtree_insert(&inttree, 1, 2);
   mu_assert_equal("lookup up 1 -> 2", gtree_searchi(&inttree, 1), 2);
   mu_assert_equal("lookup up 42 -> 0", gtree_searchi(&inttree, 1), 2);

   // test the string tree
   gtree_t strtree;

   gtree_init(&strtree, strless);

   mu_assert_equal("lookup up non-existent string", gtree_searchi(&strtree, "hello!"), 0);
   gtree_insert(&strtree, "mikio", 314);
   gtree_insert(&strtree, "stefan", 281);
   mu_assert_equal("lookup up mikio", gtree_searchi(&strtree, "mikio"), 314);
   mu_assert_equal("lookup up stefan", gtree_searchi(&strtree, "stefan"), 281);
   mu_assert_equal("lookup up karl", gtree_searchi(&strtree, "karl"), 0);

   // hm... let's test the strcmp function!
   mu_assert_equal("strcmp(abcd, abcd) == 0", strcmp("abcd", "abcd"), 0);
   mu_assert_equal("strcmp(abcd, abca) == 1", strcmp("abcd", "abcc"), 1);
   mu_assert_equal("strcmp(abcd, abcz) == -1", strcmp("abcd", "abce"), -1);   
END_CASE

BEGIN_TESTS
   mu_run_test(gtree);
END_TESTS
