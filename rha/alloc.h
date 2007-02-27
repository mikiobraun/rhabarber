/*
 * alloc - GC'd alloc
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef ALLOC_H
#define ALLOC_H

#include <stdbool.h>
#include <gc/gc.h>

#define ALLOC(type) GC_MALLOC(sizeof(struct type))
#define ALLOC_SIZE(size) GC_MALLOC(size)
#define ALLOC_RAW(size) GC_MALLOC_ATOMIC_IGNORE_OFF_PAGE(size)
#define NEW(ptr) (ptr) = GC_MALLOC(sizeof(*ptr))

extern char *gc_strdup(const char *c);
extern char *nongc_strdup(const char *c);

#endif
