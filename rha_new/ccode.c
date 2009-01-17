#include "ccode.h"

#include <stdlib.h>   // for system
#include <dlfcn.h>    // for dlopen, etc

#include "rha_types.h"  // for RHA_int (for now)
#include "messages.h"
#include "object.h"

// next step: try
// autoreconf -i
// ./configure
// make


// bundle all .o in a library


// NOTES:
// * probably the lib can not be closed if we still want to use that
// function
// * thus make a list for all lib_handles and write a function that
// closes all at the end of rhabarber...

any_t ccode(string_t codestring, any_t rtype, int narg, ...)
{
  // STEPS:
  // 1. create c-file
  // 2. compile with gcc -dynamiclib -o lib1.so lib1.c
  // 3. dlopen it
  void *lib_handle = dlopen("example.so", RTLD_LAZY);
  if (!lib_handle) rha_error("(ccode) can't open shared object");
  code_t wrapped_code;

  // the next line triggers the following compilation warning:
  // > ISO C forbids assignment between function pointer and void*
  // is that a problem of dlsym which returns a void* ?
  wrapped_code = dlsym(lib_handle, "wrapped_ccode");
  const char* error = dlerror();
  if (error) rha_error("(ccode) dlsym created error msg: %s", error);

  // 4. create builtin from wrapped_code
  any_t proto = lookup(rtype, proto_sym);
  if (!proto) rha_error("[ccode] return type must have 'proto' slot");
  ptype_t rptype = ptype(proto);
  ccode_t c = { wrapped_code, rptype };
  va_list args;
  va_start(args, narg);
  any_t f = create_ccode(c, false, narg, args);
  va_end(args);
  return f;
}


void close_all_dlibs()
{
  rha_error("not yet");
}
