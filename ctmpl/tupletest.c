#include <stdio.h>
#include "tupleint.h"
#include "listint.h"

void printint(int a)
{
  printf("[%d]",a);
}

int main(int argc, char **argv)
{
  listint_t l;
  listint_init(&l);

  for(int i = 1; i < argc; i++) {
    listint_prepend(&l, atoi(argv[i]));
  }

  listint_foreach(&l, &printint);
  printf("\n");

  tupleint_t t;

  tupleint_fromlist(&t, &l);

  listint_clear(&l);
  tupleint_foreach(&t, &printint);

  tupleint_clear(&t);
}
