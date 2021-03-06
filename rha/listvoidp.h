// Template generated by maketempl from file ../../include/ctmpl/list.tmpl
//
// maketempl has been written by Mikio Braun, 2005
// mikio@first.fraunhofer.de
//

#ifndef LISTVOIDP
#define LISTVOIDP

#include "alloc.h"
#include "voidp_defs.h"

#line 17 "../../include/ctmpl/list.tmpl"

#include <stdbool.h>

struct nodevoidp {
  struct nodevoidp *next;
  struct nodevoidp *prev;
  voidp data;
};

struct listvoidp {
  int len;
  struct nodevoidp *first;
  struct nodevoidp *last;
};

// Basic access
typedef struct listvoidp listvoidp_t;
typedef struct nodevoidp *listvoidp_iterator_t;
typedef void listvoidp_proc(voidp data);

extern void listvoidp_init(listvoidp_t *l);
extern void listvoidp_clear(listvoidp_t *l);

extern void listvoidp_append(listvoidp_t *l, voidp data);
extern void listvoidp_prepend(listvoidp_t *l, voidp data);
extern void listvoidp_appendlist(listvoidp_t *l1, listvoidp_t *l2);

extern voidp listvoidp_remove(listvoidp_t *l);
extern voidp listvoidp_pop(listvoidp_t *l);

extern voidp listvoidp_front(listvoidp_t *l);
extern voidp listvoidp_last(listvoidp_t *l);

extern bool listvoidp_isempty(listvoidp_t *l);
extern int listvoidp_length(listvoidp_t *l);

// in case == makes sense
extern void listvoidp_uniquecopyappendlist(listvoidp_t *l1, listvoidp_t *l2);
extern void listvoidp_uniqueappend(listvoidp_t *l, voidp data);
extern bool listvoidp_ismember(listvoidp_t *l, voidp data);

// Iterating
extern void listvoidp_foreach(listvoidp_t *l, listvoidp_proc *p);

extern void listvoidp_begin(listvoidp_iterator_t *i, listvoidp_t *l);
extern listvoidp_iterator_t listvoidp_iterator(listvoidp_t *l);
extern bool listvoidp_done(listvoidp_iterator_t *i); 
extern void listvoidp_next(listvoidp_iterator_t *i);
extern voidp listvoidp_get(listvoidp_iterator_t *i);
extern void listvoidp_set(listvoidp_iterator_t *i, voidp data);


#endif
