
// put here all C types that should be available inside Rhabarber

typedef void void_t;
typedef void *object_t;
typedef int symbol_t;
typedef struct { 
  void *code;       // pointer to the code
  int_t narg;       // number of arguments
  int_t argtypes[]; // types of the arguments
} fn_t;

typedef bool bool_t;
typedef int int_t;
typedef double real_t;
typedef double *mat_t;

typedef list_t list_t;  // what is the list_t?
