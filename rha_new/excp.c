#include "excp.h"
#include <stdio.h>
#include "object.h"

symbol_t msg_sym = 0;

void excp_init(object_t root, object_t module)
{
  msg_sym = symbol_new("msg");
}

object_t excp_new(string_t msg)
{
  object_t excp = new();
  assign(excp, msg_sym, WRAP_PTR(STRING_T, string_proto, msg));
  return excp;
}

void excp_show(object_t excp)
{
  object_t msg_obj = lookup(excp, msg_sym);
  if (msg_obj && (ptype(msg_obj)==STRING_T)) {
    string_t msg = UNWRAP_PTR(STRING_T, msg_obj);
    fprintf(stderr, "Caught exception: %s\n", msg);
  }
  else {
    fprintf(stderr, "Caught exception without message.\n");
  }
}
