#include "excp.h"
#include <stdio.h>
#include "object.h"
#include "utils.h"


void excp_init(object_t root, object_t module)
{
  // nothingness
}

object_t excp_new(string_t msg)
{
  object_t excp = new();
  assign(excp, msg_sym, WRAP_PTR(STRING_T, msg));
  return excp;
}

object_t excp_newf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  return excp_new(vsprint(fmt, &ap));
}

string_t excp_msg(object_t excp)
{
  object_t msg_obj = lookup(excp, msg_sym);
  if (msg_obj && (ptype(msg_obj)==STRING_T)) {
    string_t msg = UNWRAP_PTR(STRING_T, msg_obj);
    return msg;
  }
  else {
    return sprint("[error] exception raised without message!");
  }
}

void excp_show(object_t excp)
{
  fprintf(stderr, "%s\n", excp_msg(excp));
}
