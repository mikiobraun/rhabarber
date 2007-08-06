// this file is used by rha_stub.pl
// and all c-code that should be accessible in rhabarber

#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdbool.h>

// put here all C types that should be available inside Rhabarber
typedef bool bool_t;
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


//typedef ??? list_t;  // what is the list_t?
typedef struct gtuple *tuple_t;


/////////////////////////
#define VOID_T   0
#define OBJECT_T 1
#define SYMBOL_T 2
#define FN_T     3
#define BOOL_T   4
#define INT_T    5
#define REAL_T   6
#define MAT_T    7
#define TUPLE_T   8

// get the raw data and convert
//
// for example, raw(int_t, o)
#define RAW(tp, o) ((tp*)(o->raw))
#define ASSERT_RAW_NONZERO(o) assert(raw(o) != 0)

#endif

