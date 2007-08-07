// this file is used by rha_config.pl
// and all c-code that should be accessible in rhabarber

#ifndef DATATYPES_H
#define DATATYPES_H
#define rhabarber  // ignored in C

// (1) datatypes which are available in Rhabarber
#include <stdbool.h>
typedef bool rhabarber bool_t;
typedef void void_t;
typedef int symbol_t;
typedef int int_t;
typedef double real_t;
typedef char *string_t;
typedef struct rha_object *object_t;
typedef struct { 
  int_t rettype;     // return type (currently not used)
  void  *code;       // pointer to the code
  int_t narg;        // number of arguments
  int_t *argtypes;   // array of the arg types
} fn_t;
typedef double *mat_t;
typedef struct gtuple *tuple_t;

// (2) integer id for all types
#define VOID_T   0
#define OBJECT_T 1
#define SYMBOL_T 2
#define FN_T     3
#define BOOL_T   4
#define INT_T    5
#define REAL_T   6
#define MAT_T    7
#define TUPLE_T   8

// (3) prototype objects for all types
extern object_t int_proto;

// (4) core symbols
extern symbol_t quote_sym;
extern symbol_t this_sym;
extern symbol_t root_sym;
extern symbol_t local_sym;

// (5) some useful macros
// get the raw data and convert
//
// for example, raw(int_t, o)
#define RAW(tp, o) ((tp*)(o->raw))
#define ASSERT_RAW_NONZERO(o) assert(raw(o) != 0)

#endif

