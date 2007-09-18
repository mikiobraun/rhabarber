/*
 * gtuple - an fixed length array of values
 *
 * (c) 2005 by Mikio L. Braun
 */

/*
 * This file has been generated from the gtuple.tmpl template + some
 * editing
 */

#ifndef GTUPLE_H
#define GTUPLE_H

#include <stdint.h>

struct gtuple 
{
  int len;
  intptr_t *array;
};

// define types
typedef struct gtuple gtuple_t;
typedef void gtuple_proc(intptr_t v);

// prototypes
extern void      gtuple_init    (gtuple_t *t, int len);
extern void      gtuple_initto_ (gtuple_t *t, int len, intptr_t x);
extern void      gtuple_copy    (gtuple_t *t, gtuple_t *t2);
extern void      gtuple_clear   (gtuple_t *t);
extern void      gtuple_foreach (gtuple_t *t, gtuple_proc *fct);
extern int       gtuple_length  (gtuple_t *t);
extern void      gtuple_set_    (gtuple_t *t, int i, intptr_t x);
extern intptr_t  gtuple_get     (gtuple_t *t, int i);
extern intptr_t *gtuple_array   (gtuple_t *t);

#define gtuple_initto(t, len, x) gtuple_initto_(t, len, (intptr_t) x)
#define gtuple_set(t, i, x) gtuple_set_(t, i, (intptr_t)x)
#define gtuple_geti(t, i) ((int)gtuple_get(t, i))
#define gtuple_getp(t, i) ((void*)gtuple_get(t, i))

#endif
