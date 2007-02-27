#include <stdio.h>
#include "alloc.h"

#ifndef Tlessthan
#define Tlessthan(x,y) ((x) < (y))
#endif

struct Tavltree {
  struct Tavlnode *top;
};

struct Tavlnode {
  struct Tavlnode *up;
  struct Tavlnode *left;
  struct Tavlnode *right;
  int balance;
  T key;
};

typedef struct Tavltree *Tavltree_t;
typedef struct Tavlnode *Tavlnode_t;

void printTavlnode(Tavlnode_t n);
void printTavltree(Tavltree_t t);


//
// Initialization
// 
void initTavltree(Tavltree_t t)
{
  t->top = 0;
}

Tavltree_t newTavltree()
{
  Tavltree_t t = NEW(Tavltree);
  initTavltree(t);
  return t;
}

Tavlnode_t findTavltree(Tavltree_t t, T searchkey)
{
  Tavlnode_t n = t->top;
  while(n) {
    if(Tlessthan(n->key, searchkey)) n = n->left;
    else if(Tlessthan(searchkey, n->key)) n = n->right;
    else return n;
  }
  return 0;
}

// replace node pn with n with respect to its parent
void Trepatriate(Tavltree_t t, Tavlnode_t pn, Tavlnode_t n)
{
  n->up = pn->up;
  if(n->up) {
    if(pn == pn->up->left) n->up->left = n;
    else if(pn == pn->up->right) n->up->right = n;
  }
  else
    t->top = n;
}

void Trebalance(Tavltree_t t, Tavlnode_t n)
{
  if(!n->up) return;

  printf("Rebalance at %d with bal = %d, bal(pn) = %d\n", 
	 n->key, n->balance, n->up->balance);

  Tavlnode_t pn = n->up;
  if(n == pn->left) { // left child
    if(pn->balance == 1) 
      pn->balance--;
    else if(pn->balance == 0)
      Trebalance(t, pn);
    else { // pn->balance == -1
      if(n->balance == -1) { // rotation right
	Trepatriate(t, pn, n);
	pn->up = n;
	pn->left = n->right; 
	n->right = pn;
	n->balance = 0;
	pn->balance = 0;
      }
      else { // pn->balance == 1 => // double rotation left-right
	Tavlnode_t m = n->right;
	Trepatriate(t, pn, m);
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
      Trebalance(t, pn);
    else { // pn->balance == 1
      if(n->balance == 1) { // rotation left
	Trepatriate(t, pn, n);
	pn->up = n;
	pn->right = n->left; 
	n->left = pn;
	n->balance = 0;
	pn->balance = 0;
      }
      else { // pn->balance == -1 => // double rotation right-left
	Tavlnode_t m = n->left;
	Trepatriate(t, pn, m);
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

void insertTavltree(Tavltree_t t, T insertkey)
{
  if(!t->top) {
    Tavlnode_t n = NEW(Tavlnode);
    *n = (struct Tavlnode) 
      { .up = 0, .left = 0, .right = 0, .balance = 0, .key = insertkey };
    t->top = n;
    return;
  }
  Tavlnode_t n = t->top;
  Tavlnode_t in; 
  while(n) {
    if(Tlessthan(insertkey, n->key)) { 
      if(n->left) n = n->left; 
      else {
	// we have found our nice spot!
	in = NEW(Tavlnode);
	*in = (struct Tavlnode)
	  { .up = n, .left = 0, .right = 0, .balance = 0, .key = insertkey };
	n->left = in;
	if(n->balance == 1) {
	  n->balance--;
	  return; // we're done!
	}
	else {
	  n->balance--;
	  printf("Before rebalancing:");
	  printTavltree(t);
	  Trebalance(t, n);
	  return;
	}
      }
    }
    else if(Tlessthan(n->key, insertkey)) { 
      if(n->right) n = n->right;
      else {
	// we have found it!
	in = NEW(Tavlnode);
	*in = (struct Tavlnode)
	  { .up = n, .left = 0, .right = 0, .balance = 0, .key = insertkey };
	n->right = in;
	if(n->balance == -1) {
	  n->balance++;
	  return; // we're done!
	}
	else {
	  n->balance++;
	  printf("Before rebalancing:");
	  printTavltree(t);
	  Trebalance(t, n);
	  return;
	}
      }
    }
    else return; // here, we could just replace the key!
  }
}

void printTavlnode(Tavlnode_t n)
{
  if(n) {
    printf("[");printTavlnode(n->left);
    printf(" %d%c", n->key, (n->balance == 1) ? '+' : ((n->balance == -1 ? '-' : ' ')));
    printTavlnode(n->right); printf("]");
  }
}

void printTavltree(Tavltree_t t)
{ 
  if(t->top) {
    printTavlnode(t->top);
    printf("\n");
  }
  else
    printf("[empty tree]\n");
}

int main()
{
  Tavltree_t t = newTavltree();

  printTavltree(t);
  insertTavltree(t, 10); printTavltree(t);
  insertTavltree(t, 3); printTavltree(t);
  insertTavltree(t, 15); printTavltree(t);
  insertTavltree(t, 7); printTavltree(t);
  insertTavltree(t, 9); printTavltree(t);
  insertTavltree(t, 8); printTavltree(t);
}
