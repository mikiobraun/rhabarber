#include "string_fn.h"

#include <string.h>
#include "alloc.h"
#include "messages.h"

string_t string_copy(string_t other)
{
  return gc_strdup(other);
}

string_t string_concat(string_t s1, string_t s2)
{
  string_t s1s2 = ALLOC_SIZE(sizeof(char) * (strlen(s1) + strlen(s2) + 1));
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

string_t string_to_string(string_t s)
{
  // this is boring
  // however, inside rhabarber we might want to overload it...
  return s;
}

string_t string_get(string_t s, int_t i)
{
  int slen = strlen(s);
  if (i >= slen || -i > slen)
    rha_error("index out-of-bounds");
  if (i < 0) i = slen + i;
  string_t dst = ALLOC_SIZE(sizeof(char) * 2);
  strncpy(dst, s+i, 1);
  return dst;
}


string_t string_set(string_t s, int_t i, string_t v)
{
  // replace the 'i'-th position with 'v', e.g.
  //     string_set("test", 1, "a") == "tast"
  //     string_set("test", 1, "oa") == "toast"
  int slen = strlen(s);
  if (i >= slen || -i > slen)
    rha_error("index out-of-bounds");
  if (i < 0) i = slen + i;
  string_t dst;
  strncpy(dst, s, i);
  return string_concat(string_concat(dst, v), dst+(i+1));
}
