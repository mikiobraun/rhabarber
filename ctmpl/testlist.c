#include <stdio.h>
#include "listint.h"

void printint(int a)
{
  printf("%d\n", a);
}

int main(int argc, char **argv)
{
  struct listint l;

  listint_init(&l);

  printf("List is empty? -> %d\n", listint_isempty(&l));

  listint_append(&l, 3);
  listint_append(&l, 1);
  listint_append(&l, 4);

  listint_foreach(&l, &printint);

  printf("removing and prepending last element\n");
  listint_prepend(&l, listint_remove(&l));
  listint_foreach(&l, &printint);

  printf("removing and prepending last element\n");
  listint_prepend(&l, listint_remove(&l));
  listint_foreach(&l, &printint);

  printf("List is empty? -> %d\n", listint_isempty(&l));

  listint_foreach(&l, &printint);
}
