#ifndef DISPATCH_H
#define DISPATCH_H

#include "object.h"

extern object_t dispatch(object_t o, symbol_t s, list_t in);
extern object_t dispatch0(object_t o, symbol_t);
extern object_t dispatch1(object_t o, symbol_t, object_t in1);
extern object_t dispatch2(object_t o, symbol_t, object_t in1, object_t in2);
extern object_t dispatch3(object_t o, symbol_t, object_t in1, object_t in2, object_t in3);

#endif
