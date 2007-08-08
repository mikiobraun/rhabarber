 /* file: minunit.h */
// adapted from http://www.jera.com/techinfo/jtns/jtn002.html

#include <stdio.h>

#define mu_assert(message, test) do { if (!(test)) { printf("%s:%d:%s: %s", __FILE__, __LINE__, mu_name, message); assert_failed++; } else assert_passed++; } while (0)
#define mu_assert_equal(message, test, val) do { int e = (test); if (e != val) { printf("%s:%d:%s: %s, expected %d != got %d\n", __FILE__, __LINE__, mu_name, message, val, e); \
     assert_failed++; } else assert_passed++; } while (0)
#define mu_assert_equal_ptr(message, test, val) do { void* e = (test); if (e != val) { printf("%s:%d:%s: %s, expected %08x != got %08x\n", __FILE__, __LINE__, mu_name, message, val, e); \
     assert_failed++; } else assert_passed++; } while (0)
#define mu_run_test(test) do { int old_assert_failed = assert_failed; \
                               test_ ## test(); tests_run++; } while (0)

int assert_failed = 0;
int assert_passed = 0;
int tests_run = 0;

#define BEGIN_CASE(name) void test_ ## name() { char *mu_name = #name; 
#define END_CASE }

#define BEGIN_TESTS static void all_tests() {
#define END_TESTS } \
int main(int argc, char **argv) { \
     all_tests(); \
     printf("Tests run: %d\n", tests_run); \
     printf("Assertions passed: %d, failed: %d\n", assert_passed, assert_failed); \
 \
     if (! assert_failed) { \
         printf(">>> ALL TESTS PASSED <<<\n", __FILE__, __LINE__); \
     } \
     return assert_failed != 0; \
 } \

