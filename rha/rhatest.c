#include <stdio.h>
#include "alloc.h"
#include "object.h"

int main(int argc, char **argv)
{
  GC_init();

  object_t a = integer_new(42);
  object_t b = real_new(35);
  object_t c = string_new("Hello, World!\n");

  printf("a is a %s\n", type_name(object_primtype(a)));
  printf("b is a %s\n", type_name(object_primtype(b)));
  printf("c is a %s\n", type_name(object_primtype(c)));

  object_assign(c, symbol_new("theAnswer"), a);

  if( object_lookup(c, symbol_new("theAnswer")) == a ) {
    printf("the answer has been found!\n");
  }
  else {
    printf("the answer is lost!\n");
  }
}
