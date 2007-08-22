/*
 * gtree - a Generic balanced binary TREE
 *
 * written by Stefan Harmeling,
 * adapted by Mikio Braun
 */


#include "gtree.h"
#include "utilalloc.h"

/* binary gtrees implemented as red-black gtrees
 * as described in Cormen, Leiserson, Rivest: Introduction to algorithms
 *
 * written by Stefan Harmeling, 2005
 *
 * templatized by Mikio Braun
 *
 * re-edited by Mikio Braun
 *
 * and finally de-templatized by Mikio Braun :)
 */

#include <stdlib.h>
#include <stdio.h>

int gtree_lastsearchdepth;
static struct gtreenode *nil = NULL;
// all leaves point to nil
// the parent of the root is NULL

/************************************************************
 * Initialization
 */

// is this the first call of gtree_init?
void gtree_init(gtree_t *t, gtreelessfct_t *lessfct) 
{
  if (nil == NULL) {
    nil = (struct gtreenode *) util_malloc(sizeof(struct gtreenode));
    nil->color = gtree_BLACK;
  }

  // the empty gtree
  t->root = nil;
  t->size = 0;
  t->lessfct = lessfct;
#ifdef FAST_GTREE_LIMIT
  memset(t->fast_values, FAST_GTREE_LIMIT * sizeof(intptr_t), 0);
#endif
}



/************************************************************
 * gtree status queries
 */

// is the gtree empty?
bool gtree_isempty(gtree_t *t) {
  return (t->root == nil);
}


// size of the gtree
int gtree_size(gtree_t *t) {
  return t->size;
}


// is the gtree empty?
static
bool gtree_node_isempty(struct gtreenode *t) {
  return (t == nil);
}

/************************************************************
 * traverse the gtree
 */

// traverses the t in order
static
void gtree_node_traverse(struct gtreenode *t, 
			gtreetraversefct_t travfct, void *data) 
{
  if (t != nil) {
    gtree_node_traverse(t->left, travfct, data);
    travfct(t->key, t->value, data);
    gtree_node_traverse(t->right, travfct, data);
  }
}


void gtree_traverse(gtree_t *t,
			gtreetraversefct_t travfct, void *data)
{ 
  gtree_node_traverse(t->root, travfct, data); 
}

/************************************************************
 * destroy the whole gtree
 */

// clears t
static
void gtree_node_clear(struct gtreenode *t) {
  if (t != nil) {
    gtree_node_clear(t->left);
    gtree_node_clear(t->right);
    util_free(t);
  }
}


void gtree_clear(gtree_t *t) { 
  gtree_node_clear(t->root); 
  gtree_init(t, t->lessfct);
}

/************************************************************
 * searching for a key
 */ 

// find the key in x and return its value
// or NULL if not found (e.g. if x is empty)
// note that also value might be NULL
static
intptr_t gtree_node_search(struct gtreenode *x, intptr_t key) {
  gtree_lastsearchdepth = 0;
  while (x != nil) {
    if (x->gtree->lessfct(key,x->key)) x = x->left;
    else if (x->gtree->lessfct(x->key,key)) x = x->right;
    else return x->value;
    gtree_lastsearchdepth++;
  }
  return (intptr_t) NULL;  // not found
}


intptr_t gtree_search_(gtree_t *t, intptr_t key) { 
#ifdef FAST_GTREE_LIMIT
  if (key < FAST_GTREE_LIMIT)
    return t->fast_values[key];
#endif
  return gtree_node_search(t->root, key);
}

/************************************************************
 * insert a value into the gtree
 */ 
// returns the node containing the minimum of x or NULL if x is empty
static
struct gtreenode *
gtree_node_minimum(struct gtreenode *x) 
{
  if (gtree_node_isempty(x)) return NULL;
  while (x->left != nil)
    x = x->left;
  return x;
}


// returns the node containing the maximum of x or NULL if x is empty
/*
static
struct gtreenode *
gtree_maximum(struct gtreenode *x) 
{
  if (gtree_node_isempty(x)) return NULL;
  while (x->right != nil)
    x = x->right;
  return x;
}
*/

static
struct gtreenode *
gtree_node_successor(struct gtreenode *x) 
{
  if (gtree_node_isempty(x)) return NULL;
  if (x->right != nil) return gtree_node_minimum(x->right);
  struct gtreenode *y = x->p;
  while (y != NULL && x == y->right) {
    x = y;
    y = x->p;
  }
  return y;
}


/*
static
struct gtreenode *
gtree_node_predecessor(struct gtreenode *x) 
{
  if (gtree_node_isempty(x)) return NULL;
  if (x->left != nil) return gtree_maximum(x->left);
  struct gtreenode *y = x->p;
  while (y != NULL && x == y->left) {
    x = y;
    y = x->p;
  }
  return y;
}
*/


// inserts (key,value) in t, if key exists, replaces value
struct gtreenode *
gtree_node_insert_(gtree_t *t, intptr_t key, intptr_t value) 
{
  // find location to insert (key,value) into t
  struct gtreenode *y = NULL;   // NULL is the parent of t
  struct gtreenode *x = t->root;
  while (x != nil) {
    y = x;  // y is the parent of x after the loop
    if (t->lessfct(key,x->key)) x = x->left;
    else if (t->lessfct(x->key,key)) x = x->right;
    else {
      // key exist, only change its value
      x->value = value;
      return NULL;
    }
  }

  // create new node z
  struct gtreenode *z 
    = (struct gtreenode *) util_malloc(sizeof(struct gtreenode));
  z->key = key;
  z->value = value;
  z->left = nil;
  z->right = nil;
  z->p = y;
  z->gtree = t;
  if (y == NULL) t->root = z;
  else {
    if (t->lessfct(z->key,y->key)) y->left = z;
    else y->right = z;
  }
  t->size++;
  return z;
}


static
void gtree_node_leftrotate(gtree_t *t, struct gtreenode *x) {
  // assumption: x->right != nil
  struct gtreenode *y = x->right;         // set y
  x->right = y->left;   // turn y's left subgtree into x's right subgtree
  if (y->left != nil) y->left->p = x;
  y->p = x->p;          // link x's parent to y
  if (x->p == NULL) t->root = y;
  else { 
    if (x == x->p->left) x->p->left = y;
    else x->p->right = y;
  }
  y->left = x;          // put x on y's left
  x->p = y;
}


static
void gtree_node_rightrotate(gtree_t *t, struct gtreenode *x) {
  // assumption: x->left != nil
  struct gtreenode *y = x->left;         // set y
  x->left = y->right;   // turn y's right subgtree into x's left subgtree
  if (y->right != nil) y->right->p = x;
  y->p = x->p;          // link x's parent to y
  if (x->p == NULL) t->root = y;
  else { 
    if (x == x->p->right) x->p->right = y;
    else x->p->left = y;
  }
  y->right = x;          // put x on y's right
  x->p = y;
}


// inserts (key,value) into t
void gtree_insert_(gtree_t *t, intptr_t key, intptr_t value) 
{
#ifdef FAST_GTREE_LIMIT
  if (key < FAST_GTREE_LIMIT) {
    if (!t->fast_values[key])
      t->size++;
    t->fast_values[key] = value;
    return;
  }
#endif

  struct gtreenode *x = gtree_node_insert_(t,key,value);
  // to test whether red-black stuff is working replace 
  // if (0 && x!=NULL) ...
  if (x != NULL) {  // if x == NULL then x existed already
    x->color = gtree_RED;
    struct gtreenode *y;
    while (x != t->root && x->p->color == gtree_RED) {
      if (x->p == x->p->p->left) {
	y = x->p->p->right;
	if (y->color == gtree_RED) {
	  x->p->color = gtree_BLACK;
	  y->color = gtree_BLACK;
	  x->p->p->color = gtree_RED;
	  x = x->p->p;
	}
	else {
	  if (x == x->p->right) {
	    x = x->p;
	    gtree_node_leftrotate(t,x);
	  }
	  x->p->color = gtree_BLACK;
	  x->p->p->color = gtree_RED;
	  gtree_node_rightrotate(t,x->p->p);
	}
      }
      else {
	y = x->p->p->left;
	if (y->color == gtree_RED) {
	  x->p->color = gtree_BLACK;
	  y->color = gtree_BLACK;
	  x->p->p->color = gtree_RED;
	  x = x->p->p;
	}
	else {
	  if (x == x->p->left) {
	    x = x->p;
	    gtree_node_rightrotate(t,x);
	  }
	  x->p->color = gtree_BLACK;
	  x->p->p->color = gtree_RED;
	  gtree_node_leftrotate(t,x->p->p);
	}
      }
    }
    t->root->color = gtree_BLACK;
  }
}

static
void gtree_node_deletefixup(gtree_t *t, struct gtreenode *x) 
{
  struct gtreenode *w;
  while (x != t->root && x->color == gtree_BLACK) {
    if (x == x->p->left) {
      w = x->p->right;
      if (w->color == gtree_RED) {
	w->color = gtree_BLACK;
	x->p->color = gtree_RED;
	gtree_node_leftrotate(t,x->p);
	w = x->p->right;
      }
      if (w->left->color == gtree_BLACK && w->right->color == gtree_BLACK) {
	w->color = gtree_RED;
	x = x->p;
      }
      else {
	if (w->right->color == gtree_BLACK) {
	  w->left->color = gtree_BLACK;
	  w->color = gtree_RED;
	  gtree_node_rightrotate(t,w);
	  w = x->p->right;
	}
	w->color = x->p->color;
	x->p->color = gtree_BLACK;
	w->right->color = gtree_BLACK;
	gtree_node_leftrotate(t,x->p);
	x = t->root;
      }
    }
    else {
      w = x->p->left;
      if (w->color == gtree_RED) {
	w->color = gtree_BLACK;
	x->p->color = gtree_RED;
	gtree_node_rightrotate(t,x->p);
	w = x->p->left;
      }
      if (w->right->color == gtree_BLACK && w->left->color == gtree_BLACK) {
	w->color = gtree_RED;
	x = x->p;
      }
      else {
	if (w->left->color == gtree_BLACK) {
	  w->right->color = gtree_BLACK;
	  w->color = gtree_RED;
	  gtree_node_leftrotate(t,w);
	  w = x->p->left;
	}
	w->color = x->p->color;
	x->p->color = gtree_BLACK;
	w->left->color = gtree_BLACK;
	gtree_node_rightrotate(t,x->p);
	x = t->root;
      }
    }
  }
  x->color = gtree_BLACK;
}

// deletes key in t and returns its value
intptr_t gtree_delete_(gtree_t *t, intptr_t key) 
{
#ifdef FAST_GTREE_LIMIT
  if (key < FAST_GTREE_LIMIT) {
    intptr_t saved_value = t->fast_values[key];
    if (saved_value) {
      t->fast_values[key] = 0;
      --t->size;
    }
    return saved_value;
  }
#endif

  // find key in t
  struct gtreenode *z = t->root;
  while (z != nil) {
    if (t->lessfct(key,z->key)) z = z->left;
    else if (t->lessfct(z->key,key)) z = z->right;
    else break;
  }
  if (z == nil) return (intptr_t) NULL;  // not found and not deleted

  // remove z
  struct gtreenode *y;
  if (z->left == nil || z->right == nil) y = z;
  else y = gtree_node_successor(z);
  struct gtreenode *x;
  if (y->left != nil) x = y->left;
  else x = y->right;
  x->p = y->p;
  if (y->p == NULL) t->root = x;
  else {
    if (y == y->p->left) y->p->left = x;
    else y->p->right = x;
  }
  intptr_t value = z->value;
  if (y != z) {
    z->key = y->key;
    z->value = y->value;
  }
  if (y->color == gtree_BLACK) gtree_node_deletefixup(t,x);
  util_free(y);
  t->size--;
  return value;
}

//
// Iterating
//

// go to the left-most node
static
void gtree_downleft(gtree_iterator_t *i) {
  //  if (i->c == nil) return; // subgtree is empty
  while (i->c->left != nil) {
    struct gtree_iterstack *s = util_malloc(sizeof(struct gtree_iterstack));
    s->state = gtree_LEFT;
    s->prev = i->hist;
    i->hist = s;
    i->c = i->c->left;
  }
}

void gtree_begin(gtree_iterator_t *i, gtree_t *t)
{
  struct gtree_iterstack *s = util_malloc(sizeof(struct gtree_iterstack));
  s->state = gtree_LEFT;
  s->prev = 0;
  i->hist = s;
  if (t->root==nil) {
    i->c = NULL;
    return;
  }
  i->c = t->root;
  gtree_downleft(i);
#ifdef FAST_GTREE_LIMIT
  i->fast_it = 0;
  while(!t->fast_values[i->fast_it] && i->fast_it < FAST_GTREE_LIMIT)
    ++i->fast_it;
  i->t = t;
#endif
}

void gtree_next(gtree_iterator_t *i)
{
#ifdef FAST_GTREE_LIMIT
  if(i->fast_it < FAST_GTREE_LIMIT) {
    ++i->fast_it;
    while(i->fast_it < FAST_GTREE_LIMIT
	  && !i->t->fast_values[i->fast_it])
      ++i->fast_it;
    return; /* we must return in any case: When we are done the first
	       time, the gtree iterator still points to the first
	       object and we don't need to increase it! */
	       
  }
#endif

  if (i->c == nil) return;
  if (i->hist->state == gtree_LEFT) {
    // have a look at the right side
    struct gtreenode *right = i->c->right;
    if (right != nil) {
      i->hist->state = gtree_RIGHT;
      i->c = right;
      struct gtree_iterstack *s = util_malloc(sizeof(struct gtree_iterstack));
      s->state = gtree_LEFT;
      s->prev = i->hist;
      i->hist = s;
      gtree_downleft(i);
      return;
    }
  }
  // try to backtrack
  do {
    struct gtreenode *p = i->c->p;
    i->c = p;
    if (p == NULL) return; // end of the gtree
    // pop off the current state
    struct gtree_iterstack *st = i->hist;
    i->hist = st->prev;
    util_free(st);
  } while (i->hist->state == gtree_RIGHT);
}


bool gtree_done(gtree_iterator_t *i)
{
#ifdef FAST_GTREE_LIMIT
  return (i->fast_it == FAST_GTREE_LIMIT && i->c == NULL);
#else
  return (i->c == NULL);
#endif
}


intptr_t gtree_get_key_(gtree_iterator_t *i)
{
#ifdef FAST_GTREE_LIMIT
  if (i->fast_it < FAST_GTREE_LIMIT)
    return i->fast_it;
#endif
  return i->c->key;
}


intptr_t gtree_get_value_(gtree_iterator_t *i)
{
#ifdef FAST_GTREE_LIMIT
  if (i->fast_it < FAST_GTREE_LIMIT)
    return i->t->fast_values[i->fast_it];
#endif
  return i->c->value;
}
