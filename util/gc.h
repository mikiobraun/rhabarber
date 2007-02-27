/*
 * gc - garbage collector
 *
 */

#ifndef GC_H
#define GC_H

#include <stddef.h>

struct protect_struct
{
  void **var;
  struct protect_struct *next;
};

typedef struct protect_struct GC_PS;

// function prototypes
typedef void finalizer_t(void *obj);
typedef void marker_t(void *obj);

// the pointer to the last dynamic protect struct 
extern GC_PS *gc_cur_pframe;

// If you enter a function which has protected variables, invoke this
// macro at the beginning...
#define FCT_ENTER   GC_PS *gc_saved_pframe = gc_cur_pframe
// ... declare protected variables with this macro:
// (Usage: PROTECTED_VAR(mytype_t, a) = inital value)
#define PROTECTED_VAR(type, name)   type name; \
                                  GC_PS gc_##name##_ps = { &name, NULL }; \
                                  gc_cur_pframe->next = &gc_##name##_ps; \
                                  gc_cur_pframe = &gc_##name##_ps; \
                                  name
// ... finally call this when you leave 
#define FCT_LEAVE   gc_cur_pframe = gc_saved_pframe; \
                    gc_cur_pframe->next = 0

// You might also just replace return -> GC_RETURN to have an
// automatic FCT_LEAVE. However, note that return then has to be
// enclosed in a block in if, while, for, etc.
#define gc_return      FCT_LEAVE; return

/*
 * functions
 */

// initialize the garbage collector
extern void gc_init();

// allocate some memory
extern void *gc_malloc(size_t s);

// protect a global variable
extern void gc_protect_global(void **name);

// register a finalizer with an object
extern void gc_register_finalizer(void *obj, finalizer_t *f);
extern void gc_register_marker(void *obj, marker_t *m);

extern void gc_print_statistics();

// perform explicit garbage collection
extern void gc_collect();

extern void gc_mark(void *p);
#endif
