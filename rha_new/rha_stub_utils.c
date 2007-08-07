// helper function solely for 'rha_stub.c'

#include <stdarg.h>
#include "alloc.h"
#include "rha_stub_utils.h"

void add_function(object_t root, symbol_t s, int rettype, void *code, int narg, ...)
{
  // create a new object
  object_t o = new();
  setptype(o, FN_T);
  // create a struct containing all info about the builtin function
  fn_t f = ALLOC_SIZE(size_of(fn_t));
  setraw(o, (void *) f);
  f->rettype = rettype;
  f->code = code;
  f->narg = narg;
  f->argtypes = ALLOC_SIZE(narg*size_of(int_t));

  // read out the argument types
  va_list ap;
  va_start(ap, narg);
  for (int i=0; i<narg; i++)
    f->argtypes[i] = va_arg(ap, int_t);
  va_end(ap);

  // finally add it to root
  assign(root, s, o);
}

void saferaw(
