// -*- C -*-
// this file is used by rha_stub.pl
// and all c-code that should be accessible in rhabarber

//MODULES
#include "object.h"
#include "eval.h"
#include "parse.h"
#include "prules.h"
#include "core.h"
#include "symbol_fn.h"
#include "tuple_fn.h"
#include "int_fn.h"
#include "string_fn.h"

//TYPES
#include <stdbool.h>
#include "gtuple.h"
#include "glist.h"
struct rha_object;
typedef int                 _rha_ symbol_t;
typedef struct rha_object * _rha_ object_t;
typedef int                 _rha_ int_t; 
typedef bool                _rha_ bool_t;
typedef struct gtuple*      _rha_ tuple_t;
typedef struct { 
  object_t (*code)(tuple_t t);        
  int_t narg;        
  int_t *argptypes; 
}*                          _rha_ fn_t;
typedef double              _rha_ real_t;
typedef double *            _rha_ mat_t;
typedef char *              _rha_ string_t;
typedef void *              _rha_ addr_t;
typedef struct glist*       _rha_ list_t;

// for eval
typedef list_t              _rha_ call_t;


//SYMBOLS
// the most basic symbols
extern symbol_t proto_sym;
extern symbol_t quote_sym;
extern symbol_t this_sym;
extern symbol_t root_sym;
extern symbol_t local_sym;
extern symbol_t void_sym;
extern symbol_t parent_sym;
extern symbol_t literal_sym;

// special slots in root
extern symbol_t modules_sym;
extern symbol_t keywords_sym;
extern symbol_t prules_sym;

// special slots elsewhere
extern symbol_t scope_sym;
extern symbol_t argnames_sym;
extern symbol_t fnbody_sym;
extern symbol_t priority_sym;

// special symbols for parsing
extern symbol_t curlied_sym;
extern symbol_t squared_sym;
extern symbol_t rounded_sym;

//MACROS
#define UNWRAP_INT(o)    (unwrap_int(INT_T, o))
#define UNWRAP_BOOL(o)   (unwrap_int(BOOL_T, o))
#define UNWRAP_SYMBOL(o) (unwrap_int(SYMBOL_T, o))
#define UNWRAP_REAL(o)   (unwrap_double(REAL_T, o))
#define UNWRAP_PTR(pt, o) (unwrap_ptr(pt, o))

#define WRAP_INT(i)    (wrap_int(INT_T, int_proto, i))
#define WRAP_BOOL(b)   (wrap_int(BOOL_T, bool_proto, b))
#define WRAP_SYMBOL(s) (wrap_int(SYMBOL_T, symbol_proto, s))
#define WRAP_REAL(d)   (wrap_double(REAL_T, real_proto, d))
#define WRAP_PTR(pt, proto, p)    (wrap_ptr(pt, proto, p))

//PRULES
