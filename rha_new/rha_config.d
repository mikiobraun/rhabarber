// -*- C -*-
// this file is used by rha_stub.pl
// and all c-code that should be accessible in rhabarber

//MODULES
#include "object.h"
#include "eval.h"
//#include "parse.h"
//#include "bool_fn.h"

//TYPES
#include <stdbool.h>
typedef int _rha_ symbol_t;
struct rha_object;
typedef struct rha_object * _rha_ object_t;
typedef int _rha_ int_t;
typedef bool _rha_ bool_t;
#include "util/gtuple.h"
typedef struct gtuple* _rha_ tuple_t;
typedef struct { 
  object_t (*code)(tuple_t t);        // pointer to the code
  int_t narg;        // number of arguments
  int_t *argtypes; // array of the arg types
} _rha_ fn_t;
typedef double _rha_ real_t;
typedef double * _rha_ mat_t;
typedef char * _rha_ string_t;

//SYMBOLS
extern symbol_t proto_sym;
extern symbol_t quote_sym;
extern symbol_t this_sym;
extern symbol_t root_sym;
extern symbol_t local_sym;
extern symbol_t void_sym;
extern symbol_t parent_sym;

//MACROS
#define test
