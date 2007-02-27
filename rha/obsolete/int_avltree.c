#line 1 "avltree.c"
#include <stdio.h>
#include "alloc.h"

#ifndef intlessthan
#define intlessthan(x,y) ((x) < (y))
#endif

struct intavltree {
  struct intavlnode *top;
};

struct intavlnode {
  struct intavlnode *up;
  struct intavlnode *left;
  struct intavlnode *right;
  int balance;
  int key;
};

typedef struct intavltree *intavltree_t;
typedef struct intavlnode *intavlnode_t;

void printintavlnode(intavlnode_t n);
void printintavltree(intavltree_t t);


//
// Initialization
// 
void initintavltree(intavltree_t t)
{
  t->top = 0;
}

intavltree_t newintavltree()
{
  intavltree_t t = NEW(intavltree);
  initintavltree(t);
  return t;
}

intavlnode_t findintavltree(intavltree_t t, int searchkey)
{
  intavlnode_t n = t->top;
  while(n) {
    if(intlessthan(n->key, searchkey)) n = n->left;
    else if(intlessthan(searchkey, n->key)) n = n->right;
    else return n;
  }
  return 0;
}

// replace node pn with n with respect to its parent
void intrepatriate(intavltree_t t, intavlnode_t pn, intavlnode_t n)
{
  n->up = pn->up;
  if(n->up) {
    if(pn == pn->up->left) n->up->left = n;
    else if(pn == pn->up->right) n->up->right = n;
  }
  else
    t->top = n;
}

void intrebalance(intavltree_t t, intavlnode_t n)
{
  if(!n->up) return;

  printf("Rebalance at %d with bal = %d, bal(pn) = %d\n", 
	 n->key, n->balance, n->up->balance);

  intavlnode_t pn = n->up;
  if(n == pn->left) { // left child
    if(pn->balance == 1) 
      pn->balance--;
    else if(pn->balance == 0)
      intrebalance(t, pn);
    else { // pn->balance == -1
      if(n->balance == -1) { // rotation right
	intrepatriate(t, pn, n);
	pn->up = n;
	pn->left = n->right; 
	n->right = pn;
	n->balance = 0;
	pn->balance = 0;
      }
      else { // pn->balance == 1 => // double rotation left-right
	intavlnode_t m = n->right;
	intrepatriate(t, pn, m);
	pn->left = m->right;
	n->right = m->left;
	m->right = pn; pn->up = m;
	m->left = n; n->up = m;
	n->balance = -m->balance;
	pn->balance = m->balance;
	m->balance = 0;
      }
    }
  }
  else if(n == n->up->right) {
    if(pn->balance == -1) 
      pn->balance++;
    else if(pn->balance == 0)
      intrebalance(t, pn);
    else { // pn->balance == 1
      if(n->balance == 1) { // rotation left
	intrepatriate(t, pn, n);
	pn->up = n;
	pn->right = n->left; 
	n->left = pn;
	n->balance = 0;
	pn->balance = 0;
      }
      else { // pn->balance == -1 => // double rotation right-left
	intavlnode_t m = n->left;
	intrepatriate(t, pn, m);
	pn->right = m->left;
	n->left = m->right;
	m->left = pn; pn->up = m;
	m->right = n; n->up = m;
	n->balance = -m->balance;
	pn->balance = m->balance;
	m->balance = 0;
      }
    }
  }
}

void insertintavltree(intavltree_t t, int insertkey)
{
  if(!t->top) {
    intavlnode_t n = NEW(intavlnode);
    *n = (struct intavlnode) 
      { .up = 0, .left = 0, .right = 0, .balance = 0, .key = insertkey };
    t->top = n;
    return;
  }
  intavlnode_t n = t->top;
  intavlnode_t in; 
  while(n) {
    if(intlessthan(insertkey, n->key)) { 
      if(n->left) n = n->left; 
      else {
	// we have found our nice spot!
	in = NEW(intavlnode);
	*in = (struct intavlnode)
	  { .up = n, .left = 0, .right = 0, .balance = 0, .key = insertkey };
	n->left = in;
	if(n->balance == 1) {
	  n->balance--;
	  return; // we're done!
	}
	else {
	  n->balance--;
	  printf("Before rebalancing:");
	  printintavltree(t);
	  intrebalance(t, n);
	  return;
	}
      }
    }
    else if(intlessthan(n->key, insertkey)) { 
      if(n->right) n = n->right;
      else {
	// we have found it!
	in = NEW(intavlnode);
	*in = (struct intavlnode)
	  { .up = n, .left = 0, .right = 0, .balance = 0, .key = insertkey };
	n->right = in;
	if(n->balance == -1) {
	  n->balance++;
	  return; // we're done!
	}
	else {
	  n->balance++;
	  printf("Before rebalancing:");
	  printintavltree(t);
	  intrebalance(t, n);
	  return;
	}
      }
    }
    else return; // here, we could just replace the key!
  }
}

void printintavlnode(intavlnode_t n)
{
  if(n) {
    printf("[");printintavlnode(n->left);
    printf(" %d%c", n->key, (n->balance == 1) ? '+' : ((n->balance == -1 ? '-' : ' ')));
    printintavlnode(n->right); printf("]");
  }
}

void printintavltree(intavltree_t t)
{ 
  if(t->top) {
    printintavlnode(t->top);
    printf("\n");
  }
  else
    printf("[empty tree]\n");
}

int main()
{
  intavltree_t t = newintavltree();

  printintavltree(t);
  insertintavltree(t, 10); printintavltree(t);
  insertintavltree(t, 3); printintavltree(t);
  insertintavltree(t, 15); printintavltree(t);
  insertintavltree(t, 7); printintavltree(t);
  insertintavltree(t, 9); printintavltree(t);
  insertintavltree(t, 8); printintavltree(t);
}
