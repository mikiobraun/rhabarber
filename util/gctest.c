#include "gc.h"

void *allocsome()
{
  FCT_ENTER;

  PROTECTED_VAR(void*, p) = 0; 

  p = gc_malloc(1048576 + 1048576/2);

  gc_collect();
  gc_print_statistics();

  FCT_LEAVE;
  return p;
}

void *globalp = 0;

int main(int argc, char **argv)
{
  gc_init();

  gc_protect_global(&globalp);

  globalp = allocsome();
  allocsome();

  gc_print_statistics();
  gc_collect();
  gc_print_statistics();
  
  allocsome();

  gc_print_statistics();
  gc_collect();
  gc_print_statistics();
}
