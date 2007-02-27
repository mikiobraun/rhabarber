#include <stdio.h>
#include "hashintint.h"
#include "hashstringint.h"

void printint(int a)
{
  printf("%d", a);
}

void printstring(char *s)
{
  printf("%s", s);
}

int main(int argc, char **argv)
{
  {
    struct hashstringint h;
    
    hashstringint_init(&h);
    
    hashstringint_assign(&h, "Mikio", 42105642);
    hashstringint_assign(&h, "Katrin", 89790341);
    
    hashstringint_dump(&h, &printstring, &printint);
  
    printf("Looking up %s -> %d\n", "Mikio", hashstringint_lookup(&h, "Mikio"));

    hashstringint_clear(&h);
  }

  {
    struct hashintint h;

    hashintint_init(&h);

    for(int i = 0; i < 10; i++) {
      hashintint_assign(&h, i, i);
    }

    hashintint_dump(&h, &printint, &printint);
  }
}
