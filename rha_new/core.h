#ifndef CORE_H
#define CORE_H

#include "rha_types.h"

extern object_t _rha_ if_fn(object_t this, bool_t cond, object_t then_code, object_t else_code);
extern object_t _rha_ return_fn(object_t);
extern object_t _rha_ deliver_fn(object_t);
extern void     _rha_ break_fn(object_t);
extern void     _rha_ throw_fn(object_t);
extern object_t _rha_ do_fn(object_t this, object_t code);
extern object_t _rha_ while_fn(object_t this, object_t cond, object_t code);
extern object_t _rha_ for_fn(object_t this, symbol_t var, object_t container, object_t code);
extern object_t _rha_ try_fn(object_t this, object_t tryblock, symbol_t catchvar, object_t catchblock);
extern addr_t   _rha_ addr_fn(object_t);
extern void     _rha_ tic_fn();
extern double   _rha_ toc_fn();
extern void     _rha_ exit_fn(int_t);

#endif
