#include "string_fn.h"

#include <string.h>
#include "alloc.h"

string_t string_copy(string_t other)
{
  string_t s = ALLOC_SIZE(strlen(other) + 1);
  strcpy(s, other);
  return s;
}

string_t string_concat(string_t s1, string_t s2)
{
  string_t s1s2 = ALLOC_SIZE(strlen(s1) + strlen(s2) + 1);
  strcat(strcpy(s1s2, s1), s2);
  return s1s2;
}


int_t string_len(string_t s)
{
  // note that the return type of 'strlen' is 'size_t'
  return (int_t) strlen(s);
}

bool_t string_equalequal(string_t s1, string_t s2)
{
  return 0==strcmp(s1, s2);
}
