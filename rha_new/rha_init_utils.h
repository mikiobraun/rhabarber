// helper functions solely for 'rha_stub.c'

#include "object.h"

extern void add_datatype(object_t root, symbol_t s, int_t type);
extern void add_function(object_t root, symbol_t s, int rettype, void *code, int narg, ...);

