#include "lexer_tr.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "alloc.h"
#include "object.h"
#include "utils.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"

struct lexer_s
{
  RHA_OBJECT;
  object_t start_symbol;  // start symbol of grammar rules
  list_tr lhs;     // left hand sides of grammar rules
  list_tr rhs;     // right hand sides of grammar rules
  list_tr prec;    // precedences of grammar rules
  list_tr left;    // left-associativeness of grammar rules
  list_tr action;  // syntax analysis table, part 'action'
  list_tr jump;    // syntax analysis table, part 'jump'
};

#define CHECK(x) CHECK_TYPE(lexer, x)

primtype_t lexer_type = 0;
extern primtype_t plain_type;

void lexer_init(void)
{
  if (!lexer_type) {
    lexer_type = primtype_new("lexer", sizeof(struct lexer_s), lexer_obj);
    (void) lexer_new();
  }
}


lexer_tr lexer_new()
{
  lexer_tr p = object_new(lexer_type);
  p->start_symbol = string_new();  // start symbol of grammar rules
  p->lhs;     // left hand sides of grammar rules
  p->rhs;     // right hand sides of grammar rules
  p->prec;    // precedences of grammar rules
  p->left;    // left-associativeness of grammar rules
  p->action;  // syntax analysis table, part 'action'
  p->jump;    // syntax analysis table, part 'jump'

  return i;
}


lexer lexer_add(lexer_tr p, list_tr l)
{
  CHECK(p);
  return i->value;
}


lexer lexer_parse(lexer_tr i, string_tr)
{
  CHECK(p);
  return i->value;
}


string_t lexer_to_string(lexer_tr p)
{
  CHECK(p);
  //return sprint("%d", p);
}


#undef CHECK
