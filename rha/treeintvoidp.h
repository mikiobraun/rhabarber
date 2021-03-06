// Template generated by maketempl from file ../../include/ctmpl/tree.tmpl
//
// maketempl has been written by Mikio Braun, 2005
// mikio@first.fraunhofer.de
//

#ifndef TREEINTVOIDP
#define TREEINTVOIDP

#include "alloc.h"
#include "intdefs.h"
#include "voidp_defs.h"

#line 18 "../../include/ctmpl/tree.tmpl"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


struct treeintvoidp_treenode {
  enum {treeintvoidp_RED, treeintvoidp_BLACK} color;
  int key;
  voidp value;
  struct treeintvoidp_treenode *left;
  struct treeintvoidp_treenode *right;
  struct treeintvoidp_treenode *p;      // parent
};


typedef struct treeintvoidp_ {
  struct treeintvoidp_treenode *root;
  int size;
} treeintvoidp_t;

struct treeintvoidp_iterstack {
  enum {treeintvoidp_LEFT, treeintvoidp_RIGHT} state;
  struct treeintvoidp_iterstack *prev;
};

typedef struct {
  struct treeintvoidp_treenode *c;
  struct treeintvoidp_iterstack *hist;
} treeintvoidp_iterator_t;

extern int treeintvoidp_lastsearchdepth;

extern void treeintvoidp_init(treeintvoidp_t *t);
// creates the empty tree

extern void treeintvoidp_print(treeintvoidp_t *t);
// prints the t in order

extern void treeintvoidp_clear(treeintvoidp_t *t);
// clears t

extern bool treeintvoidp_isempty(treeintvoidp_t *t);
// is the tree empty?

extern int treeintvoidp_size(treeintvoidp_t *t);
// size of the tree

extern voidp treeintvoidp_search(treeintvoidp_t *x, int key);
// finds the key in x and return its value
// or NULL if not found (e.g. if x is empty)
// note that also value might be NULL

extern void treeintvoidp_insert(treeintvoidp_t *t, int key, voidp value);
// inserts (key,value) into t

extern voidp treeintvoidp_delete(treeintvoidp_t *t, int key);
// deletes key in t and returns its value

// Iterating
extern void treeintvoidp_begin(treeintvoidp_iterator_t *i, treeintvoidp_t *t);
extern bool treeintvoidp_done(treeintvoidp_iterator_t *i);
extern void treeintvoidp_next(treeintvoidp_iterator_t *i);
extern int treeintvoidp_get_key(treeintvoidp_iterator_t *i);
extern voidp treeintvoidp_get_value(treeintvoidp_iterator_t *i);


#endif
