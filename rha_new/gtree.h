/*
 * gtree - a Generic balanced binary TREE
 *
 * written by Stefan Harmeling,
 * adapted by Mikio Braun
 */

#ifndef UTIL_GTREE_H
#define UTIL_GTREE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//#define FAST_GTREE_LIMIT 12

struct gtreenode {
  enum {gtree_RED, gtree_BLACK} color;
  intptr_t key;
  intptr_t value;
  struct gtreenode *left;
  struct gtreenode *right;
  struct gtreenode *p;      // parent
  struct gtree *gtree; 
};

// should check for a < b
typedef bool gtreelessfct_t(intptr_t a, intptr_t b);

// called for traversing a gtree
typedef void gtreetraversefct_t(intptr_t key, intptr_t val, void *data);

typedef struct gtree {
  struct gtreenode *root;
  int size;
  gtreelessfct_t *lessfct;
#ifdef FAST_GTREE_LIMIT
  intptr_t fast_values[FAST_GTREE_LIMIT];
#endif
} gtree_t;

struct gtree_iterstack {
  enum {gtree_LEFT, gtree_RIGHT} state;
  struct gtree_iterstack *prev;
};

typedef struct {
  struct gtreenode *c;
  struct gtree_iterstack *hist;
#ifdef FAST_GTREE_LIMIT
  int fast_it;
  struct gtree *t; 
#endif
} gtree_iterator_t;

extern int gtree_lastsearchdepth;

// creates the empty gtree
extern void gtree_init(gtree_t *t, gtreelessfct_t *lessfct);

// clears t
extern void gtree_clear(gtree_t *t);

// is the gtree empty?
extern bool gtree_isempty(gtree_t *t);

// size of the gtree
extern int gtree_size(gtree_t *t);

// finds the key in x and return its value
// or NULL if not found (e.g. if x is empty)
// note that also value might be NULL
extern intptr_t gtree_search_(gtree_t *x, intptr_t key);

// inserts (key,value) into t
extern void gtree_insert_(gtree_t *t, intptr_t key, intptr_t value);

// deletes key in t and returns its value
extern intptr_t gtree_delete_(gtree_t *t, intptr_t key);

// Iterating
extern void gtree_begin(gtree_iterator_t *i, gtree_t *t);
extern bool gtree_done(gtree_iterator_t *i);
extern void gtree_next(gtree_iterator_t *i);
extern intptr_t gtree_get_key_(gtree_iterator_t *i);
extern intptr_t gtree_get_value_(gtree_iterator_t *i);

// Versions with implicit casting
#define gtree_insert(t, k, v) (gtree_insert_(t, (intptr_t) (k), (intptr_t) (v)))

#define gtree_searchi(t, k) ((int)  gtree_search_(t, (intptr_t) (k) ))
#define gtree_searchp(t, k) ((void*)gtree_search_(t, (intptr_t) (k) ))
#define gtree_deletei(t, k) ((int) gtree_delete_(t, (intptr_t) (k) ))
#define gtree_deletep(t, k) ((void*) gtree_delete_(t, (intptr_t) (k) ))
#define gtree_get_keyi(i) ((int)gtree_get_key_(i))
#define gtree_get_keyp(i) ((void*)gtree_get_key_(i))
#define gtree_get_valuei(i) ((int)gtree_get_value_(i))
#define gtree_get_valuep(i) ((void*)gtree_get_value_(i))



#endif
