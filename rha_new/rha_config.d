// this file is used by rha_stub.pl
// and all c-code that should be accessible in rhabarber

//MODULES
#include "object.h"
#include "eval.h"

//TYPES
#include <stdbool.h>
typedef int _rha_ symbol_t;
typedef void* _rha_ object_t;
typedef struct { 
  int_t rettype;     // return type (currently not used)
  void *code;        // pointer to the code
  int_t narg;        // number of arguments
  int_t *argtypes[]; // array of the arg types
} _rha_ fn_t;
typedef bool _rha_ bool_t;
typedef int _rha_ int_t;
typedef double _rha_ real_t;
typedef double * _rha_ mat_t;

//SYMBOLS
extern symbol_t proto_sym;
extern symbol_t quote_sym;
extern symbol_t this_sym;
extern symbol_t root_sym;
extern symbol_t local_sym;
