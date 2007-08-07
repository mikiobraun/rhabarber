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
typedef void rhabarber void_t;
typedef void* rhabarber object_t;
typedef int rhabarber symbol_t;
typedef struct { 
  int_t rettype;     // return type (currently not used)
  void *code;        // pointer to the code
  int_t narg;        // number of arguments
  int_t *argtypes[]; // array of the arg types
} rhabarber fn_t;
typedef bool rhabarber bool_t;
typedef int rhabarber int_t;
typedef double rhabarber real_t;
typedef double * rhabarber mat_t;

//SYMBOLS
extern symbol_t quote_sym;
extern symbol_t this_sym;
extern symbol_t root_sym;
extern symbol_t local_sym;
