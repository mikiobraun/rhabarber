#ifndef UTIL_ALLOC_H
#define UTIL_ALLOC_H

#include <stdlib.h>

typedef void *mallocfct_t(size_t s);
typedef void freefct_t(void *ptr);

extern mallocfct_t *util_malloc;
extern freefct_t *util_free;

#endif
