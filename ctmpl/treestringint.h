// Template generated by maketempl from file tree.tmpl
//
// maketempl has been written by Mikio Braun, 2005
// mikio@first.fraunhofer.de
//

#ifndef TREESTRINGINT
#define TREESTRINGINT

#include "stringdefs.h"

#line 18 "tree.tmpl"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


struct treestringint_treenode {
  enum {RED, BLACK} color;
  string key;
  int value;
  struct treestringint_treenode *left;
  struct treestringint_treenode *right;
  struct treestringint_treenode *p;      // parent
};


typedef struct {
  struct treestringint_treenode *root;
  int size;
} treestringint_t;

extern int treestringint_lastsearchdepth;

extern void treestringint_init(treestringint_t *t);
// creates the empty tree

extern void treestringint_print(treestringint_t *t);
// prints the t in order

extern void treestringint_clear(treestringint_t *t);
// clears t

extern bool treestringint_isempty(treestringint_t *t);
// is the tree empty?

extern int treestringint_search(treestringint_t *x, string key);
// finds the key in x and return its value
// or NULL if not found (e.g. if x is empty)
// note that also value might be NULL

extern void treestringint_insert(treestringint_t *t, string key, int value);
// inserts (key,value) into t

extern int treestringint_delete(treestringint_t *t, string key);
// deletes key in t and returns its value


#endif
