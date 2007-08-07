// WARNING: don't edit here!  
// This file has been automatically generated by 'rha_config.pl'.
// Instead edit 'rha_config.d'.

#ifndef DATATYPES_H
#define DATATYPES_H
#define rhabarber

// (1) datatypes which are available in Rhabarber
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



// (2) primtype id for all types
#define SYMBOL_T_t 1 
#define OBJECT_T_t 2 
#define FN_T_t 3 
#define BOOL_T_t 4 
#define INT_T_t 5 
#define REAL_T_t 6 
#define MAT_T_t 7 


// (3) prototype objects for all types
extern object_t symbol_proto;
extern object_t object_proto;
extern object_t fn_proto;
extern object_t bool_proto;
extern object_t int_proto;
extern object_t real_proto;
extern object_t mat_proto;


// (4) symbols
extern object_t symbol_sym;
extern object_t object_sym;
extern object_t fn_sym;
extern object_t bool_sym;
extern object_t int_sym;
extern object_t real_sym;
extern object_t mat_sym;


// (5) some useful macros
// get the raw data and convert
//
// for example, raw(int_t, o)
#define RAW(tp, o) ((tp*)(o->raw))
#define ASSERT_RAW_NONZERO(o) assert(raw(o) != 0)

#endif
