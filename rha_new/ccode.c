#include "ccode.h"

#include <stdlib.h>   // for system
#include <dlfcn.h>    // for dlopen, etc

#include "rha_types.h"  // for RHA_int (for now)
#include "messages.h"
#include "object.h"

// gcc -dynamiclib -o lib1.so lib1.c

// next step: try
// autoreconf -i
// ./configure
// make
//gcc -dynamiclib -std=gnu99 -Wall -pedantic -fno-common -g -O2 -framework Python  -o rhabarber alloc.o bool_fn.o ccode.o core.o cstream.o debug.o eval.o excp.o fmt.o glist.o gtree.o gtuple.o int_fn.o list_fn.o mat_fn.o messages.o object.o parse.o prules.o python_fn.o real_fn.o rhabarber.o rha_lexer.o rha_parser.o string_fn.o symbol_fn.o symtable.o tuple_fn.o utilalloc.o utils.o -lfl rha_init.o -lgsl -lcblas -lreadline -lgc -o example.so example.c


// bundle all .o in a library


// NOTES:
// * probably the lib can not be closed if we still want to use that
// function
// * thus make a list for all lib_handles and write a function that
// closes all at the end of rhabarber...

// any_t ccode(string_t codestring, any_t returntype, any_t argtype)
// {
//   // STEPS:
//   // 1. create c-file
//   // 2. compile with gcc -dynamiclib -o lib1.so lib1.c
//   // 3. dlopen it
//   void *lib_handle = dlopen("example.so", RTLD_LAZY);
//   if (!lib_handle) rha_error("(ccode) can't open shared object");
//   builtin_t wrapped_code;
//  
//   // the next line triggers the following compilation warning:
//   // > ISO C forbids assignment between function pointer and void*
//   // is that a problem of dlsym which returns a void* ?
//   wrapped_code = (builtin_t) dlsym(lib_handle, "wrapped_ccode");
//   const char* error = dlerror();
//   if (error) rha_error("(ccode) dlsym created error msg: %s", error);
//  
//   // 4. create builtin from wrapped_code
//   any_t f = create_builtin(wrapped_code, false, 1, RHA_int);
//   return f;
// }

any_t ccode(string_t codestring, any_t returntype, any_t argtype)
{
  // STEPS:
  // 1. create c-file
  // 2. compile with gcc -dynamiclib -o lib1.so lib1.c
  // 3. dlopen it
  void *lib_handle = dlopen("example.so", RTLD_LAZY);
  if (!lib_handle) rha_error("(ccode) can't open shared object");
  ccode_t wrapped_code;

  // the next line triggers the following compilation warning:
  // > ISO C forbids assignment between function pointer and void*
  // is that a problem of dlsym which returns a void* ?
  wrapped_code = dlsym(lib_handle, "wrapped_ccode");
  const char* error = dlerror();
  if (error) rha_error("(ccode) dlsym created error msg: %s", error);

  // 4. create builtin from wrapped_code
  enum ptypes ptype = 0;
  any_t f = create_ccode(wrapped_code, ptype, false, 1, argtype);
  return f;
}


void close_all_dlibs()
{
  rha_error("not yet");
}
