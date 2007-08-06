// this file is used by rha_stub.pl
// and all c-code that should be accessible in rhabarber

//INCLUDES
#include "object.h"
#include "eval.h"
#include "core.h"
#include "bool_fn.h"
#include "int_fn.h"
#include "real_fn.h"
#include "mat_fn.h"

//DATATYPES
#include <stdbool.h>
typedef void void_t;
typedef void *object_t;
typedef int symbol_t;
typedef struct { 
  int_t rettype;     // return type (currently not used)
  void *code;        // pointer to the code
  int_t narg;        // number of arguments
  int_t *argtypes[]; // array of the arg types
} fn_t;
typedef bool bool_t;
typedef int int_t;
typedef double real_t;
typedef double *mat_t;

typedef ??? list_t;  // what is the list_t?
typedef ??? tuple_t;

