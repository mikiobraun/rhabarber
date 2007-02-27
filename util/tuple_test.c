#include <stdio.h>
#include "gtuple.h"

void printgtuple(gtuple_t *t)
{
  for(int i = 0; i < gtuple_length(t); i++)
    printf("%s ", (char*)gtuple_getp(t, i));
  printf("\n");
}

int main(int argc, char **argv)
{
  gtuple_t t;
  gtuple_init(&t, argc - 1);

  for(int i = 1; i < argc; i++)
    gtuple_set(&t, i-1, argv[i]);

  printgtuple(&t);

  gtuple_clear(&t);
}
