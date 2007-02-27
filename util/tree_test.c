#include <stdio.h>

#include "gtree.h"

int cmpfct(intptr_t a, intptr_t b)
{
  return strcmp((char*)a, (char*)b) < 0;
}

int main(int argc, char **argv)
{
  struct {
    char *name;
    int number;
  } phonebook[] = {
    { "mikio", 42105642 },
    { "stefan", 325412454 },
    { "klaus", 324125 },
    { "guido", 572572 }
  };

  gtree_t t;

  gtree_init(&t, cmpfct);

  for(int i = 0; i < 4; i++)
    gtree_insert(&t, (void*)(phonebook[i].name), phonebook[i].number);

  gtree_print(&t);

  for(int i = 0; i < 4; i++)
    printf("%s's nummer: %d\n", phonebook[i].name, 
	   gtree_searchi(&t, phonebook[i].name));

  gtree_clear(&t);
}
