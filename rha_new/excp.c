#include "excp.h"
#include <stdio.h>
#include "object.h"
#include "utils.h"


void excp_init(any_t root, any_t module)
{
  // nothingness
}

any_t excp_new(string_t msg)
{
  any_t excp = new();
  assign(excp, msg_sym, WRAP_PTR(_STRING_T, msg));
  return excp;
}

any_t excp_newf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  return excp_new(vsprint(fmt, &ap));
}

string_t excp_msg(any_t excp)
{
  any_t msg_obj = lookup(excp, msg_sym);
  if (msg_obj && (ptype(msg_obj)==_STRING_T)) {
    string_t msg = UNWRAP_PTR(_STRING_T, msg_obj);
    return msg;
  }
  else {
    return sprint("[error] exception raised without message!");
  }
}

void excp_show(any_t excp)
{
  fprintf(stderr, "%s\n", excp_msg(excp));
}
