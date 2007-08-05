
#include <stdarg.h>
#include "object.h"


object_t eval(object_t env, object_t o)
{
  rha_error("not yet");
}



void *get_n_check(object_t o, int_t ptype)
{
  if (primtype(o) != ptype)
    rha_error("argument primtype missmatch\n");
  return raw(o);
}

object_t call_C_fun(fn_t f, int narg, ...) 
{
  // more arguments needed?  increase here!
  void *arg0, *arg1, *arg2, *arg3, *arg4, *arg5;

  // do the <stdarg.h> magic
  va_list ap;
  va_start(ap, narg);
  if (narg>0) arg0 = get_n_check(va_arg(ap, object_t), f->argtypes[0]);
  if (narg>1) arg1 = get_n_check(va_arg(ap, object_t), f->argtypes[1]);
  if (narg>2) arg2 = get_n_check(va_arg(ap, object_t), f->argtypes[2]);
  if (narg>3) arg3 = get_n_check(va_arg(ap, object_t), f->argtypes[3]);
  if (narg>4) arg4 = get_n_check(va_arg(ap, object_t), f->argtypes[4]);
  if (narg>5) arg5 = get_n_check(va_arg(ap, object_t), f->argtypes[5]);
  if (narg>6) rha_error("not implemented: too many argument");
  va_end(ap);

  // finally call 'f'
  switch (narg) {
  case 0: return ((void *(*)()) f->code)();
  case 1: return ((void *(*)(void *)) f->code)(arg0);
  case 2: return ((void *(*)(void *, void *)) f->code)(arg0, arg1);
  case 3: return ((void *(*)(void *, void *, void *)) f->code)(arg0, arg1, arg2);
  case 4: return ((void *(*)(void *, void *, void *, void *)) f->code)(arg0, arg1, arg2, arg3);
  case 5: return ((void *(*)(void *, void *, void *, void *, void *)) f->code)(arg0, arg1, arg2, arg3, arg4);
  }
}
