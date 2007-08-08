#include "utilalloc.h"

#include <stdlib.h>

mallocfct_t *util_malloc = malloc;
freefct_t *util_free = free;
