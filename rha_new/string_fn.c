#include "string_fn.h"

#include <string.h>
#include "alloc.h"

string_t string_append(string_t s1, string_t s2)
{
  char *s1s2 = ALLOC_SIZE(strlen(s1) + strlen(s2) + 1);
  strcat(strcpy(s1s2, s1), s2);
  return s1s2;
}


int_t string_len(string_t s)
{
  // note that the return type of 'strlen' is 'size_t'
  return (int_t) strlen(s);
}
