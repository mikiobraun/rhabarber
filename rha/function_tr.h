#ifndef FUNCTION_TR_H
#define FUNCTION_TR_H

#include "object.h"
#include "symbol_tr.h"
#include "tuple_tr.h"
#include "tuplesymbol.h"

typedef struct function_s *function_tr;
extern primtype_t function_type;
extern void            function_init(void);
extern function_tr     function_new(tuple_tr t, int numargs, object_t code, object_t env);
extern function_tr     function_new_prule(tuple_tr t, int numargs, object_t code, object_t env, double priority);
extern function_tr     function_new_macro(tuple_tr t, int numargs, object_t code, object_t env);

extern method int      function_numargs(function_tr f);
extern method object_t function_arg(function_tr f, int i);
extern string_t        function_argname(function_tr f, int i);
extern method object_t function_code(function_tr f);
extern method object_t function_env(function_tr f);

extern void            function_setenv(function_tr f, object_t env);
extern void            function_setcode(function_tr f, object_t code);

extern method string_t function_to_string(function_tr f);

#endif
