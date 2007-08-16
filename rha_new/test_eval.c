#include "minunit.h"
#include <stdarg.h>

#include "rha_types.h"
#include "eval.h"
#include "symbol_fn.h"
#include "tuple_fn.h"

symbol_t quote_sym, local_sym, this_sym, parent_sym;
object_t int_proto, symbol_proto;

void rha_error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);

  exit(0);
}

object_t times_two_fct(tuple_t args)
{
  int_t a = UNWRAP_INT(tuple_get(args, 1));
  printf("Yay! The two times %d is %d\n", a, 2*a);
  int_t two_a = 2*a;
  return WRAP_INT(two_a);
}

BEGIN_CASE(eval)
     // defining symbols
     quote_sym = symbol_new("quote");
     local_sym = symbol_new("local");
     this_sym = symbol_new("this");
     parent_sym = symbol_new("parent");

     // just check that symbol_new is not completely broken
     mu_assert_equal("quote_sym should be 1", quote_sym, 1);
     mu_assert_equal("local_sym should be 2", local_sym, 2);
     mu_assert_equal("this_sym should be 3", this_sym, 3);

     // set up some objects
     object_t root = new();

     object_t seventeen = WRAP_INT(17);
     object_t three = WRAP_INT(3);

     symbol_t xsym = symbol_new("x");
     symbol_t ysym = symbol_new("y");

     object_t x = WRAP_SYMBOL(xsym);
     object_t y = WRAP_SYMBOL(ysym);

     assign(root, xsym, seventeen);
     assign(root, ysym, three);

     // test for literals
     mu_assert_equal_ptr("literals should just evaluate to themselves", eval(root, seventeen), seventeen);

     // test for lookups
     mu_assert_equal_ptr("symbol 'x' should evaluate to 17", eval(root, x), seventeen);

     // test for quotations
     tuple_t qt = tuple_new(2);
     tuple_set(qt, 0, WRAP_SYMBOL(quote_sym));
     tuple_set(qt, 1, x);

     mu_assert_equal_ptr("quoted symbols should not evaluate ", eval(root, WRAP_PTR(TUPLE_T, NULL, qt)), x);

     // test for C function calls
     int_t times_two_args[1] = { INT_T };
     function_t times_two_fn = { times_two_fct, 1, times_two_args };
     symbol_t times_two = symbol_new("times_two");

     assign(root, times_two, WRAP_PTR(FN_T, NULL, &times_two_fn));

     tuple_t fncall = tuple_new(2);
     tuple_set(fncall, 0, WRAP_SYMBOL(times_two));
     tuple_set(fncall, 1, WRAP_INT(42));

     mu_assert_equal("evaluating the fn gives a FN_T", ptype(eval(root, tuple_get(fncall, 0))), FN_T);

     mu_assert_equal("calling times_two", UNWRAP_INT(eval(root, WRAP_PTR(TUPLE_T, NULL, fncall))), 84);

     //
     // testing rhabarber functions - with doubling, of course
     //
     // times_two_rha(x) = times_two(x)
     //
     object_t times_two_rha = new();
     int one = 1;
     assign(times_two_rha, symbol_new("numargs"), WRAP_INT(one));

     // set up function body, basically "times_two(x)"
     tuple_t fnbody = tuple_new(2);
     tuple_set(fnbody, 0, WRAP_SYMBOL(times_two));
     tuple_set(fnbody, 1, x);
     assign(times_two_rha, symbol_new("fnbody"), WRAP_PTR(TUPLE_T, NULL, fnbody));

     // set up argnames "(x)"
     tuple_t argnames = tuple_new(1);
     tuple_set(argnames, 0, x);

     assign(times_two_rha, symbol_new("argnames"), WRAP_PTR(TUPLE_T, NULL, argnames));

     // set up scope (set scope to root)
     assign(times_two_rha, symbol_new("scope"), root);

     // set up calling expression "(times_two_rha 42)"
     tuple_t fncall_rha = tuple_new(2);
     tuple_set(fncall_rha, 0, times_two_rha);
     tuple_set(fncall_rha, 1, WRAP_INT(42));
     
     mu_assert_equal("calling times_two as rha function", UNWRAP_INT(eval(root, WRAP_PTR(TUPLE_T, NULL, fncall_rha))), 84);
END_CASE

BEGIN_TESTS
mu_run_test(eval);
END_TESTS
