#include "string_tr.h"

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "alloc.h"
#include "symbol_tr.h"
#include "int_tr.h"
#include "builtin_tr.h"
#include "eval.h"

#include "fmt.h"

struct string_s
{
  RHA_OBJECT;
  int length;
  string_t text;
};

#define CHECK(x) CHECK_TYPE(string, x)

primtype_t string_type = 0;


void string_init(void)
{
  if (!string_type) {
    string_type = primtype_new("string", sizeof(struct string_s));
    (void) string_new("");
  }
}


string_tr string_new(string_t text)
{
  string_tr s = object_new(string_type);
  s->length = strlen(text);
  s->text = gc_strdup(text);
  return s;
}


string_tr string_copy(string_tr s)
{
  return string_new(string_get(s));
}


string_t string_get(string_tr s)
{
  CHECK(s);
  return s->text;
}


int string_length(string_tr s)
{
  CHECK(s);
  return s->length;
}


string_t string_plus_string(string_t s1, string_t s2)
{
  char *s1s2 = ALLOC_SIZE(strlen(s1) + strlen(s2) + 1);
  strcat(strcpy(s1s2, s1), s2);
  return s1s2;
}


bool string_less_string(string_t s1, string_t s2)
{
  return (strcmp(s1, s2) < 0);
}


bool string_greater_string(string_t s1, string_t s2)
{
  return (strcmp(s1, s2) > 0);
}


bool string_equal_string(string_t s1, string_t s2)
{
  return (strcmp(s1, s2) == 0);
}


string_t string_to_string(string_tr s)
{
  CHECK(s);
  return string_get(s);
}


#undef CHECK
