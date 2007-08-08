#include "minunit.h"
#include <string.h>

#include "rha_types.h"
#include "rha_lexer.h"
#include "rha_parser.tab.h"
#include "symbol_fn.h"
#include "object.h"

extern int yylex();

YYSTYPE yylval;

object_t bool_proto, symbol_proto, real_proto, string_proto, int_proto;
symbol_t parent_sym;

int yywrap()
{
  return 0;
}

BEGIN_CASE(rha_lexer)
     beginstringparse("123 1.342 abcdef \"hello!\" +*+-");

     mu_assert_equal("parsing 123", yylex(), INT);
     mu_assert("token should be 123", UNWRAP_INT(yylval.obj) == 123);
     mu_assert_equal("parsing 1.342", yylex(), REAL);
     mu_assert("token should be 1.342", UNWRAP_REAL(yylval.obj) == 1.342);
     mu_assert_equal("parsing abcdef", yylex(), SYMBOL);
     mu_assert("name should be abcdef", strcmp(symbol_name(UNWRAP_SYMBOL(yylval.obj)), "abcdef") == 0);
     mu_assert_equal("parsing \"hello!\"", yylex(), STRING);
     mu_assert_equal_str("string should be \"hello!\"", UNWRAP_PTR(STRING_T, yylval.obj), "hello!");
     mu_assert_equal("parsing +*+-", yylex(), SYMBOL);
     mu_assert("name should be +*+-", strcmp(symbol_name(UNWRAP_SYMBOL(yylval.obj)), "+*+-") == 0);

     endparse();
END_CASE

BEGIN_TESTS
mu_run_test(rha_lexer);
END_TESTS


