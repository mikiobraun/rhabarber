#include <stdio.h>
#include "glist.h"

void printglist(glist_t *l)
{
  glist_iterator_t li;
  for(glist_begin(&li, l); !glist_done(&li); glist_next(&li))
    printf("%s ", (char*)glist_getp(&li));
  printf("\n");
}

int main(int argc, char **argv)
{
  glist_t l;

  glist_init(&l);
  for(int i = 1; i < argc; i++)
    glist_append(&l, argv[i]);
  printglist(&l);
  glist_clear(&l);
}
