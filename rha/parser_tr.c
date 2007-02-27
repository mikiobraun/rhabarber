/* // current problem: */
/* //    @ <- # */
/* There is absolutely no warranty for GDB.  Type "show warranty" for details. */
/* This GDB was configured as "i686-pc-cygwin"... */
/* (gdb) b rhabarber.c:135 */
/* Breakpoint 1 at 0x40127c: file rhabarber.c, line 135. */
/* (gdb) r */
/* Starting program: /home/harmeli/rhabarber/rha/rhabarber.exe  */
/* Delicious rhabarber! */
/* --RHA has been conceived by Stefan Harmeling and Mikio Braun. */
/* --This interpreter was written by Mikio Braun and Stefan Harmeling, 2005. */
/* rha[1]$ p=parser.new() */

/* Breakpoint 1, read_eval_loop (env=0x100549a0) at rhabarber.c:135 */
/* (gdb) c */
/* Continuing. */
/* [parser] */
/* rha[2]$ p.addrule(quote, "@", "#", 5, 0==0) */

/* Breakpoint 1, read_eval_loop (env=0x100549a0) at rhabarber.c:135 */
/* (gdb) c */
/* Continuing. */
/* rha[3]$ p.addrule(fn(x,y)x+y, "#", "#+#", 3, 0==1) */

/* Breakpoint 1, read_eval_loop (env=0x100549a0) at rhabarber.c:135 */
/* (gdb) c */
/* Continuing. */
/* rha[4]$ p.addrule(fn(x,y,z)x+y+z, "#", "if # @ @", 1, 0==1) */

/* Breakpoint 1, read_eval_loop (env=0x100549a0) at rhabarber.c:135 */
/* (gdb) c */
/* Continuing. */
/* rha[5]$ p.table() */

/* Breakpoint 1, read_eval_loop (env=0x100549a0) at rhabarber.c:135 */
/* (gdb) c */
/* Continuing. */
/* state   $      #      +      @      if      */
/*     0   -/-    s/1    -/-    -/-    s/4    */
/*     1   r/0    r/0    s/2    r/0    r/0    */
/*     2   -/-    s/3    -/-    -/-    -/-    */
/*     3   r/1    r/1    s/2    r/1    r/1    */
/*     4   -/-    s/5    -/-    -/-    -/-    */
/*     5   -/-    -/-    s/2    s/6    s/4    */
/*     6   -/-    s/1    -/-    s/7    s/4    */
/*     7   r/2    s/1    r/2    r/2    s/4    */
/* rha[6]$ p.keywords() */

/* Breakpoint 1, read_eval_loop (env=0x100549a0) at rhabarber.c:135 */
/* (gdb) c */
/* Continuing. */
/* + */
/* if */
/* rha[7]$ p.parse("if 23 34 45") */


// todo: if we add and remove a rule, the terminals/nonterminals will remain in the symbol table,
//       this should be changed

// todo: find loops, see beginning of parser.py

#include "parser_tr.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <gc/gc.h>

#include "alloc.h"
#include "object.h"
#include "utils.h"
#include "messages.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "plain_tr.h"
#include "string_tr.h"
#include "tuple_tr.h"
#include "rhalexerpure.h"

#include "listobject.h"
#include "tupleobject.h"
#include "listint.h"         // for the stack of states
#include "tupleint.h"
#include "listvoidp.h"
#include "tuplevoidp.h"
#include "treeintstring.h"
#include "treestringint.h"
#include "treeintint.h"
#include "treeintvoidp.h"

struct parser_s
{
  RHA_OBJECT;
  bool dirty;
  bool verbose;

  int startsymbol;     // start symbol of grammar rules
  listobject_t lfunc;  // for each rule one function
  listvoidp_t lrules;  // collect rules here
  listvoidp_t lrulestr;// list of strings of the rules (for verbose == true)
  tuplevoidp_t trulestr;// tuple of strings of the rules (for verbose == true)
  int nextkey;         // next key for the following two symbol tables
  treestringint_t st;  // symbol table for keywords
  treeintint_t nt;     // tree mapping keys of nonterminals to zeros (used to check whether a key is a non-terminal)

  tuplevoidp_t pftree; // the prefix tree
  tupleint_t rultree;  // for each rule the corresponding complete node

  tupleobject_t tfunc; // solidified functions
  tuplevoidp_t trules; // solidified rules for faster parsing
  treeintint_t action; // syntax analysis table, part 'action'
  treeintint_t jump;   // syntax analysis table, part 'jump'
};

// note on lrules/trules
// each tupleint contains [0] prec, [1]left, [2]lhs, [3] 0, [4..]rhs
// fun is the function to call

// note on action/jump:
// the key will be calculated by the state number and the character code:
//     key = state * 1024 + code

#define CHECK(x) CHECK_TYPE(parser, x)

primtype_t parser_type = 0;
extern primtype_t plain_type;

void parser_init(void)
{
  if (!parser_type) {
    parser_type = primtype_new("parser", sizeof(struct parser_s));
    (void) parser_new();
  }
}


parser_tr parser_new()
{
  parser_tr p = object_new(parser_type);
  listobject_init(&p->lfunc);
  listvoidp_init(&p->lrules);
  listvoidp_init(&p->lrulestr);
  treestringint_init(&p->st);
  treeintint_init(&p->nt);
  treestringint_insert(&p->st, "#", 1);
  treeintint_insert(&p->nt, 1, 1);  // the value must be unequal to zero
  p->nextkey = 2;   // 0 is for '$' (not that dollar sign itself is not used here in the code, thus it can be used for rules), 
                    // 1 if for '#' (expressions/literals)
  p->startsymbol = 1;  // i.e. '#' is start symbol
  p->dirty = true;
  p->verbose = true;
  return p;
}


int scan_lhs(parser_tr p, string_tr lhs)
{
  CHECK(p);
  p->dirty = true;

  // scan lhs
  begin_scanning_string(string_get(lhs));
  int w = get_next_token();
  string_t value;
  if (w == 1)
    value = string_get(yypurelval);  // identifier
  else 
    rha_error("Identifier/keyword expected.\n");

  // is there another token?
  w = get_next_token();
  end_scanning();
  if (w != 0) { // expect EOF, i.e. 0
    rha_error("Left-hand-side/non-terminals for grammar rules must only contain one token.\n");
  }

  // check that value is new keyword or known non-terminal
  int key = treestringint_search(&p->st, value);
  if (key == 0) { // new token
    key = p->nextkey++;
    treestringint_insert(&p->st, value, key);
  }
  // list it as a non-terminal
  treeintint_insert(&p->nt, key, 1);
  return key;
}


bool is_keyword(parser_tr p, int key)
{
  return treeintint_search(&p->nt, key) == 0;
}


void parser_keywords(parser_tr p)
{
  // print all keywords
  treestringint_iterator_t it; // loop over all keywords
  for (treestringint_begin(&it, &p->st);
       !treestringint_done(&it);
       treestringint_next(&it)) {
    int key = treestringint_get_value(&it);
    if (is_keyword(p, key)) {
      printf("%s\n", treestringint_get_key(&it));
    }
  }
}


void parser_verbose(parser_tr p)
{
  p->verbose = !p->verbose;
  if (p->verbose) printf("verbose = true\n");
  else printf("verbose = false\n");
}


void parser_startsymbol(parser_tr p, string_tr lhs)
{
  int key = scan_lhs(p, lhs);
  p->startsymbol = key;
}


void parser_addrule(parser_tr p, object_t fun, string_tr lhs, string_tr rhs, int_tr prec, bool_tr left)
{
  CHECK(p);
  p->dirty = true;
  listobject_append(&p->lfunc, fun);

  listvoidp_append(&p->lrulestr, (void*) string_plus_string(string_get(lhs), string_plus_string(" <- ", string_get(rhs))));

  // scan lhs
  int lhskey = scan_lhs(p, lhs);
  
  // scan rhs
  begin_scanning_string(string_get(rhs));   // set up the lexer
  int key, w;
  listint_t lrule;
  listint_init(&lrule);
  while ((w = get_next_token())) {
    if (w == 1) { // identifier/keyword
      // fill symbol table
      string_t value = string_get(yypurelval);
      //printf("%s\n", value);
      key = treestringint_search(&p->st, value);
      if (key == 0) {
	key = p->nextkey++;
	treestringint_insert(&p->st, value, key);
      }
    }
    else if (w == 2) rha_error("Error scanning rule: no literals allowed.\n");
    else rha_error("Error scanning rule: unknown token.\n");
    listint_append(&lrule, key);
  }
  end_scanning();   // finish the lexer

  // add lhs, prec and left to the lrule
  listint_prepend(&lrule, 0);  // put a zero to mark the beginning
  listint_prepend(&lrule, lhskey);
  listint_prepend(&lrule, bool_get(left)?1:0);
  listint_prepend(&lrule, int_get(prec));
  
  // built 'tupleint trule' by solidifying lrule
  tupleint_t *trule; NEW(trule);
  tupleint_init(trule, listint_length(&lrule));
  listint_iterator_t it;
  int i = 0;
  for (listint_begin(&it, &lrule); 
       !listint_done(&it);
       listint_next(&it), i++) {
    tupleint_set(trule, i, listint_get(&it));
  }
  
  // finally, add the rule to the list of rules
  listvoidp_append(&p->lrules, trule);
}


void parser_list(parser_tr p)
{
  CHECK(p);

  // print the list of rules
  listvoidp_iterator_t it1;
  listvoidp_iterator_t it2;
  int i = 0;
  printf("     prec  left  rule\n");
  for (listvoidp_begin(&it1, &p->lrules), listvoidp_begin(&it2, &p->lrulestr);
       !listvoidp_done(&it1) && !listvoidp_done(&it2); 
       listvoidp_next(&it1), listvoidp_next(&it2), i++) {
    tupleint_t *trule = listvoidp_get(&it1);
    int prec = tupleint_get(trule, 0);
    int left = tupleint_get(trule, 1);
    string_t str = listvoidp_get(&it2);
    printf("%3d  %4d  %4d  %s\n", i, prec, left, str);
  }
}


void parser_changerule(parser_tr p, int i, object_t fun, string_tr lhs, string_tr rhs, int_tr prec, bool_tr left)
{
}


void parser_removerule(parser_tr p, int i)
{
}


// node of the prefix tree
typedef struct pftreenode_ {
  listvoidp_t childs;  // point to the children's pftreenodes
  listint_t rules;     // which rules belong to this node
  int lastsymbol;      // the last symbol of the current prefix
  int index;           // the index of the corresponding state
} pftreenode_t;


pftreenode_t *pftree_extend(listvoidp_t *lpftree, int token, int index)
     // creates a new node for the prefix tree and appends it to the
     // list of all nodes
{
  pftreenode_t *node;
  NEW(node);
  listvoidp_init(&node->childs);
  listint_init(&node->rules);
  node->lastsymbol = token;
  node->index = index;
  listvoidp_append(lpftree, node); // extend the node list for later iterations
  return node;
}


pftreenode_t *choose_child(pftreenode_t *tn, int token)
{
  // loop over the children of tn and look for token
  listvoidp_iterator_t it;
  for (listvoidp_begin(&it, &tn->childs);
       !listvoidp_done(&it);
       listvoidp_next(&it)) {
    pftreenode_t *child = listvoidp_get(&it);
    if (child->lastsymbol == token) return child;
  }
  return 0;  // not found
}


void create_prefix_tree(parser_tr p)
{
  pftreenode_t *root, *tn, *child;

  listvoidp_t *lpftree; NEW(lpftree);  // a list of all nodes that is
                              // turned into the tuple to be returned
  listvoidp_init(lpftree);
  int index = 0;  // index must count parallel to adding to lpftree
  root = pftree_extend(lpftree, 0, index++);
  // hereby, token 0 corresponds to '$', the start token

  // next add an artificial node for the start symbol
  child = pftree_extend(lpftree, p->startsymbol, index++);
  listvoidp_append(&root->childs, child);

  // initialize the tuple that maps the rule indices to the
  // corresponding nodes in the prefix tree
  tupleint_init(&p->rultree, tuplevoidp_length(&p->trules));

  // loop over all rhs
  for (int i=0; i<tuplevoidp_length(&p->trules); i++) {
    tupleint_t *rule = tuplevoidp_get(&p->trules, i);
    // walk along the tree so far and extend it if necessary
    listint_append(&root->rules, i);
    tn = root;
    for (int j = 4; j < tupleint_length(rule); j++) {
      int token = tupleint_get(rule, j);
      child = choose_child(tn, token);
      if (!child) {
	child = pftree_extend(lpftree, token, index++);
	listvoidp_append(&tn->childs, child);
      }
      // add rule number to the current node
      listint_append(&child->rules, i);
      tn = child;
    }
    tupleint_set(&p->rultree, i, tn->index);
  }
  // finally, turn the list into a tuple
  tuplevoidp_init(&p->pftree, listvoidp_length(lpftree));
  listvoidp_iterator_t it;
  int i = 0;
  for (listvoidp_begin(&it, lpftree);
       !listvoidp_done(&it);
       listvoidp_next(&it)) {
    tuplevoidp_set(&p->pftree, i++, listvoidp_get(&it));
  }
}


int get_prec(tuplevoidp_t *trules, int index)
{
  tupleint_t *rule = tuplevoidp_get(trules, index);
  return tupleint_get(rule, 0);
}

int get_left(tuplevoidp_t *trules, int index)
{
  tupleint_t *rule = tuplevoidp_get(trules, index);
  return tupleint_get(rule, 1);
}

int get_lhs(tuplevoidp_t *trules, int index)
{
  tupleint_t *rule = tuplevoidp_get(trules, index);
  return tupleint_get(rule, 2);
}

void complete_prefix_tree(parser_tr p)
{  
  // for a syntax analysis table there are some types of connections missing:
  // (i)  for each rule A->alpha with alpha not beginning with A
  //      all prefixes pointing at A should link to alpha, continue this for all
  //      non-terminals at the beginning of alpha
  // (ii) for each rule A->alpha
  //      all prefixes that contain maximally beginnings of alpha
  //      should link to the remaining part of alpha
    
  pftreenode_t *home, *node, *tn, *child;
  pftreenode_t *root = tuplevoidp_get(&p->pftree, 0);
  listvoidp_t *fs;  // pointer to follow set

  // (i)
  // create for each non-terminal a follow set
  treeintvoidp_t kids;       // the followsets of the nonterminals
  treeintvoidp_init(&kids);
  {
    treestringint_iterator_t it; // loop over all keywords
    for (treestringint_begin(&it, &p->st);
	 !treestringint_done(&it);
	 treestringint_next(&it)) {
      int key = treestringint_get_value(&it);
      if (!is_keyword(p, key)) {  // only for non-terminals, i.e. not for keywords
	NEW(fs);
	listvoidp_init(fs);
	treeintvoidp_insert(&kids, key, fs);
      }
    }
  }
  // initially fill the followsets "kids" only according to rules
  listvoidp_iterator_t it;
  for (listvoidp_begin(&it, &root->childs); 
       !listvoidp_done(&it); 
       listvoidp_next(&it)) {
    child = listvoidp_get(&it);
    listint_iterator_t jt;
    for (listint_begin(&jt, &child->rules);
	 !listint_done(&jt);
	 listint_next(&jt)) {
      // looping over all children of the root and all its associated rules
      int index = listint_get(&jt); // the number of the rule
      int nt = get_lhs(&p->trules, index); // nonterminal on the lhs
      // get the followset nt
      fs = treeintvoidp_search(&kids, nt);
      assert(fs!=0);  // all follow sets should exist already
      if (nt != child->lastsymbol) {
	// add the new index
	listvoidp_append(fs, child);
      }
    }
  }
  // next close the kids under substitution as expressed by the rules
  bool changed = true;
  while (changed) {
    changed = false;
    for (int i=0; i<tuplevoidp_length(&p->trules); i++) {
      tupleint_t *rule = tuplevoidp_get(&p->trules, i);
      int nt = tupleint_get(rule, 2);                    // nonterminal on the lhs
      listvoidp_t *kid = treeintvoidp_search(&kids, nt); // followset of nt
      assert(kid!=0);
      int old = listvoidp_length(kid);                   // to monitor changes
      int key = tupleint_get(rule, 4);                   // second symbol of rhs, first is '$'
      if (!is_keyword(p, key)) {   // key is non-terminal
	listvoidp_t *kid2 = treeintvoidp_search(&kids, key);
	if (!kid2) rha_error("Can't find follow set.\n");
	listvoidp_uniquecopyappendlist(kid, kid2);
	if (old < listvoidp_length(kid)) changed = true;
      }
    }
  }
  // search for non-terminal symbols in the tree and add the possible
  // follow states (stored in kids)
  for (int i=0; i<tuplevoidp_length(&p->pftree); i++) {
    node = tuplevoidp_get(&p->pftree, i);
    int key = node->lastsymbol;
    if (!is_keyword(p, key)) { // key is non-terminal
      listvoidp_t *kid = treeintvoidp_search(&kids, key);
      listvoidp_uniquecopyappendlist(&node->childs, kid);
    }
  }

  // (ii) [note: current implementation is not optimal, since
  // overlapping rules are searched separately.  a solution would be
  // some recursive implementation...]

  // look for all partly matched rule-beginnings along the tree
  for (int i=0; i<tuplevoidp_length(&p->trules); i++) {
    tupleint_t *rule = tuplevoidp_get(&p->trules, i);
    // each rule has a standard place in the tree starting at the root,
    // look for other places in the tree where the rule starts as well
    // and find the place where it diverges
    
    // find standard place called 'home'
    int nextsym = 4;  // index counting through the rule symbols
    int key = tupleint_get(rule, nextsym++);
    home = choose_child(root, key);
    int lenrule = tupleint_length(rule);  // length of the current rule + 4;
    assert(home!=0);  // otherwise rule is missing in prefix tree
    // find matching places in the tree
    for (int j=0; j<tuplevoidp_length(&p->pftree); j++) {
      node = tuplevoidp_get(&p->pftree, j);
      // reset the branch in the tree for the rulex
      tn = home;
      nextsym = 5;
      // as long as the current node matches go deeper, if we are at
      // the end of the rule (nextsym>=lenrule) we do nothing, this
      // corresponds to always reducing the longer rule if possible
      while (tn->lastsymbol == node->lastsymbol && nextsym<lenrule) {
	key = tupleint_get(rule, nextsym++);
	pftreenode_t *tnold = tn;
	tn = choose_child(tnold, key);
	assert(tn!=0);  // since we ensure 'nextsym<lenrule', there must be some child
	child = choose_child(node, key);
	if (!child) {
	  // no matching child found, so add a link for the connection so far
	  listvoidp_uniqueappend(&node->childs, tn);
	}
      }
    }   
  }
}


#define UNDEF  0
#define ACCEPT 1
#define SHIFT  2
#define REDUCE 3


/*
bool is_member(int x, listint_t *l)
{
  listint_iterator_t it;
  for (listint_begin(&it, l);
       !listint_done(&it);
       listint_next(&it)) {
    if (x==listint_get(&it)) return true;
  }
  return false;
}
*/


bool subset(listint_t *l1, listint_t *l2)
     // is l1 a subset of l2?
{
  listint_iterator_t it;
  // are all elements of l1 members of l2?
  for (listint_begin(&it, l1);
       !listint_done(&it);
       listint_next(&it)) {
    if (!listint_ismember(l2, listint_get(&it)))
      return false;
  }
  return true;
}


int node_cmp(pftreenode_t *node1, pftreenode_t *node2)
     // are node1 and node2 in the same branch of the tree?
     //    no -> return 0
     //    yes:
     // who is closer to the root 
     //    node1  -> return 1
     //    node2  -> return 2
{
  // solution:
  // rule set of node1:  r1
  // rule set of node2:  r2
  // index of node1: i1
  // index of node2: i2
  //   r2 < r1   ->   return 1     '<' is proper subset
  //   r1 < r2   ->   return 2
  // else                          '<' the usual less than
  //   i2 < i1   ->   return 2
  //   i1 < i2   ->   return 1
  // else return 0;

  bool s1 = subset(&node2->rules, &node1->rules);  // subset or equal
  bool s2 = subset(&node1->rules, &node2->rules);
  if (s1 && !s2) return 1;
  else if (!s1 && s2) return 2;
  else if (!s1 && !s2) return 0;
  else if (node2->index < node1->index) return 2;
  else return 1;
}


void create_sat(parser_tr p)
{
  pftreenode_t *node, *child;
  treeintint_init(&p->action);
  treeintint_init(&p->jump);

  // step 0: add reduce actions for each rule and all keywords + '$'
  for (int i=0; i<tupleint_length(&p->rultree); i++) {
    // loop over all states that can reduce
    int state = tupleint_get(&p->rultree, i);
    int w = 0; // zero corresponds to '$'
    treeintint_insert(&p->action, state*1024+w, REDUCE);
    treeintint_insert(&p->jump, state*1024+w, i);
    // NOTE: jump contains here the index of the reduced rule

    treestringint_iterator_t it; // loop over all keywords
    for (treestringint_begin(&it, &p->st);
	 !treestringint_done(&it);
	 treestringint_next(&it)) {
      w = treestringint_get_value(&it);
      treeintint_insert(&p->action, state*1024+w, REDUCE);
      treeintint_insert(&p->jump, state*1024+w, i);
      // NOTE: jump contains here the index of the reduced rule
    }
  }

  // step 1: add shift actions, hereby overwriting some reduces
  for (int state=0; state<tuplevoidp_length(&p->pftree); state++) {
    // loop through all nodes in the prefix tree (graph)
    node = tuplevoidp_get(&p->pftree, state);
    listvoidp_iterator_t it; // loop over all children
    for (listvoidp_begin(&it, &node->childs);
	 !listvoidp_done(&it);
	 listvoidp_next(&it)) {
      child = listvoidp_get(&it);
      int w = child->lastsymbol;
      int goal = child->index;
      treeintint_insert(&p->action, state*1024+w, SHIFT);
      treeintint_insert(&p->jump, state*1024+w, goal);
    }
  }

  // step 2: add accept action
  // commented out because from now on we use a different stopping mechanism, because grammars like
  //    @ <- #
  //    # <- #+#
  //    # <- if # @ @
  // with starting symbol # don't work for parsing "if # # #", 
  /*
  int state = 1;  // the artificial node for the start symbol
  int w = 0;      // corresponding to '$'
  treeintint_insert(&p->action, state*1024+w, ACCEPT);
  treeintint_insert(&p->jump, state*1024+w, -1);  // NOTE: not used
  */

  // step 3: resolve shift/reduce conflicts
  for (int i=0; i<tupleint_length(&p->rultree); i++) {
    // loop over all states that can reduce
    int state = tupleint_get(&p->rultree, i);
    node = tuplevoidp_get(&p->pftree, state);
    listvoidp_iterator_t it; // loop over all children
    for (listvoidp_begin(&it, &node->childs);
	 !listvoidp_done(&it);
	 listvoidp_next(&it)) {
      child = listvoidp_get(&it);
      int w = child->lastsymbol;
      // to resolve the shift/reduce conflict we need to compare
      // 'node' and 'child' according to the less-than relation
      // induced by the initial prefix tree
      int cmp = node_cmp(node, child);
      if (cmp == 1) {
	// case 0:   'node' is closer to the root than 'child' in the tree structure.
	//      node is a prefix or part of some longer rule,
	//      policy: always follow the longer rule
	//      this solves the 'dangling else' problem
	//   thus do nothing, i.e. keep a shift
      }
      else if (cmp == 2) {
	// case 1:  'node' is further away from root than 'child' in the tree structure.
	//         we are jumping back into the same rule
	//         use associativity to decide ('left')
	if (get_left(&p->trules, i)==1) {
	  // change from shift to reduce
	  treeintint_insert(&p->action, state*1024+w, REDUCE);
	  treeintint_insert(&p->jump, state*1024+w, i);
	}
      }
      else if (cmp == 0) {
	// case 2:  'node' and 'child' are not comparable in the original tree structure.
	//          we are jumping somewhere else, check precedence
	int pri = get_prec(&p->trules, i);
	bool allsmaller = true;
	bool alllarger = true;
	// try to beat minp and maxp with the prec of child
	listint_iterator_t it; // loop over all children
	for (listint_begin(&it, &child->rules);
	     !listint_done(&it);
	     listint_next(&it)) {
	  int pr = get_prec(&p->trules, listint_get(&it));
	  if (pr >= pri) allsmaller = false;
	  if (pr <= pri) alllarger = false;
	}
	if (allsmaller) {
	  // case 2.1: all precs are smaller than prec[i]
	  // change action from shift to reduce
	  treeintint_insert(&p->action, state*1024+w, REDUCE);
	  treeintint_insert(&p->jump, state*1024+w, i);
	}
	else if (alllarger) {
	  // case 2.2: all precs are larger than prec[i]
	  // keep the shift for w, thus do nothing
	}
	else {
	  // case 2.3: precedences are not comparable
	  // thus unresolvable shift/reduce conflict
	  string_t msg = "undecidable shift/reduce conflict resolved to shift (default).\n";
	  rha_warning(msg);
	}
      }
      else {
	assert(0);  // some bug in node_cmp
      }
    }
  }
}


void parser_update(parser_tr p)
{
  CHECK(p);
  int i;

  // solidify the lists
  {
    tuplevoidp_init(&p->trulestr, listvoidp_length(&p->lrulestr));
    listvoidp_iterator_t it;
    i = 0;
    for (listvoidp_begin(&it, &p->lrulestr); 
	 !listvoidp_done(&it); 
	 listvoidp_next(&it), i++) {
      tuplevoidp_set(&p->trulestr, i, listvoidp_get(&it));
    }
  }
  {
    tuplevoidp_init(&p->trules, listvoidp_length(&p->lrules));
    listvoidp_iterator_t it;
    i = 0;
    for (listvoidp_begin(&it, &p->lrules); 
	 !listvoidp_done(&it); 
	 listvoidp_next(&it), i++) {
      tuplevoidp_set(&p->trules, i, listvoidp_get(&it));
    }
  }
  {
    tupleobject_init(&p->tfunc, listobject_length(&p->lfunc));
    listobject_iterator_t it;
    i = 0;
    for (listobject_begin(&it, &p->lfunc); 
	 !listobject_done(&it); 
	 listobject_next(&it), i++) {
      tupleobject_set(&p->tfunc, i, listobject_get(&it));
    }
  }

  // calculate the prefix tree
  create_prefix_tree(p);
  complete_prefix_tree(p);  // note: tree becomes a graph

  // recalculate the syntax analysis tables: action and jump
  create_sat(p);
  p->dirty = false;
}


string_t str_int(int i)
{
  int buflen = 256;
  char *buffer = ALLOC_SIZE( buflen + 1 );
  buffer[0] = (char) 0;
  if (sprintf(buffer, "%d ", i) < 0) {
    rha_error("Problem printing stack.\n");
  }
  return buffer;
}

string_t str_stack(listint_t *stack)
{
  int buflen = 256;
  char *buffer = ALLOC_SIZE( buflen + 1 );  
  buffer[0] = (char) 0;
  strncat(buffer, "[ ", buflen - strlen(buffer));
  listint_iterator_t it; // loop over all children
  for (listint_begin(&it, stack);
       !listint_done(&it);
       listint_next(&it)) {
    int i = listint_get(&it);
    strncat(buffer, str_int(i), buflen - strlen(buffer));
  }
  strncat(buffer, "]", buflen - strlen(buffer));
  return buffer;
}


void print_state(int ac, int ju)
{
  if      (ac==ACCEPT) fprintf(stderr, " a/%-3d ", ju);
  else if (ac==SHIFT)  fprintf(stderr, " s/%-3d ", ju);
  else if (ac==REDUCE) fprintf(stderr, " r/%-3d ", ju);
  else if (ac==0)      fprintf(stderr, " -/-   ");
  else                 fprintf(stderr, " err   ");
}


void parser_table(parser_tr p)
{
  CHECK(p);
  if (p->dirty) parser_update(p);
  
  fprintf(stderr, "state   $      ");
  treestringint_iterator_t it; // loop over all keywords
  for (treestringint_begin(&it, &p->st);
       !treestringint_done(&it);
       treestringint_next(&it)) {
    string_t iden = treestringint_get_key(&it);
    // NOTE: jump contains here the index of the reduced rule
    fprintf(stderr, "%-7s", iden);
  }
  fprintf(stderr, "\n");

  int w, ac, ju;
  for (int state=0; state<tuplevoidp_length(&p->pftree); state++) {
    fprintf(stderr, "%5d  ", state);
    w = 0; // zero corresponds to '$'
    ac = treeintint_search(&p->action, state*1024+w);
    ju = treeintint_search(&p->jump, state*1024+w);
    print_state(ac, ju);
    for (treestringint_begin(&it, &p->st);
	 !treestringint_done(&it);
	 treestringint_next(&it)) {
      w = treestringint_get_value(&it);
      ac = treeintint_search(&p->action, state*1024+w);
      ju = treeintint_search(&p->jump, state*1024+w);
      print_state(ac, ju);
    }
    fprintf(stderr, "\n");
  }
}


int token2content(parser_tr p, int w)
{
  if (w == 0) { // EOF
    return 0;   // zero corresponds to '$'
  }
  else if (w == 1) { // identifier/keyword
    // is the identifier/keyword known to the parser as a terminal symbol?
    string_t str = string_get(yypurelval);
    int key = treestringint_search(&p->st, str);
    if (key > 0) {
      // check further if key is really a terminal and not a non-terminal symbol
      // non-terminals is the string to be parsed are handled like usual identifier without special meaning
      if (is_keyword(p, key)) {
	// for keywords return their key
	return key;  // this is always != 1, because key==1 is in the tree p->nt, see parser_new()
      }
    }
    // not found or non-terminal, thus identifier, return literal terminal which is '#', i.e. 1
    yypurelval = symbol_new(str);  // transform string to symbol
    return 1;
  }
  else if (w == 2) { // literal
    return 1; // == treestringint(&p->st, "#");
  }
  else {
    rha_error("Parse error: unknown token.\n");
  }
  assert(0); // never reach this point
  return 0;
}


object_t parser_parse(parser_tr p, string_tr s)
{
  CHECK(p);
  if (p->dirty) parser_update(p);

  // set up the lexer
  begin_scanning_string(string_get(s));

  object_t parsetree = list_new();  // collects the parsing result

  int rn, terminal, oldstate;

  int buflen = 256;
  char *msg = ALLOC_SIZE( buflen + 1 );
  // empty msg
  for (int i=0; i<buflen+1; i++) msg[i] = ' ';
  msg[0] = (char) 0;  msg[buflen] = (char) 0;

  // the stack keeps the states of the FSA
  listint_t stack;
  listint_init(&stack);
  int state = 0;   // the initial state
  listint_append(&stack, state);  // push 
  int ac = UNDEF;  // undefined action
  int w = token2content(p, get_next_token());
  while (1) {
    if (p->verbose) {
      // empty msg
      for (int i=0; i<buflen+1; i++) msg[i] = ' ';
      msg[0] = (char) 0;  msg[buflen] = (char) 0;
      // fill msg with info
      strncat(msg, str_stack(&stack), buflen-strlen(msg));
      msg[strlen(msg)] = ' ';
      msg[60] = 0;
    }
    
    // can we accept now?
    if (w==0 && listint_length(&stack)==1 && state==1) {
      if (p->verbose) {
	strncat(msg, " accept\n", buflen-strlen(msg));
	fprintf(stderr, msg);
      }
      break;
    }

    // action being in state seeing w
    ac = treeintint_search(&p->action, state*1024+w);
    if (ac == UNDEF) {
      rha_error("Parse error.\n");
    }
    else if (ac == SHIFT) {
      if (p->verbose) {
	strncat(msg, " shift ", buflen-strlen(msg));
      }
      state = treeintint_search(&p->jump, state*1024+w); // new state
      listint_append(&stack, state);                  // push new state
      // semantical stuff comes now
      if ( w == 0 ) { // EOF
	rha_error("Parse error: can't shift because EOF.\n");
      }
      else if ( w == 1 ) { // identifier/literal
	list_append(parsetree, yypurelval);
      }
      else if ( w > 1 ) { // keyword
	list_append(parsetree, 0);
      }
      else {
	rha_error("Parse error: unknown token.\n");
      }
      if (p->verbose) {
	strncat(msg, object_to_string(yypurelval), buflen-strlen(msg));
	strncat(msg, string_plus_string("  ( ", string_plus_string(str_int(w), ")")), buflen-strlen(msg));
      }
      w = token2content(p, get_next_token());    // next token
    }
    else if (ac == REDUCE) {
      rn = treeintint_search(&p->jump, state*1024+w); // the rule number to reduce
      if (p->verbose) {
	strncat(msg, " reduce ", buflen-strlen(msg));
	strncat(msg, (string_t) tuplevoidp_get(&p->trulestr, rn), buflen-strlen(msg));
      }
      tupleint_t *rule = tuplevoidp_get(&p->trules, rn);    // the rule itself
      int rulelen = tupleint_length(rule) - 4;   // length of the rule's rhs
      terminal = tupleint_get(rule, 2);          // the rule's lhs
      // pop reduced states off the stack
      object_t fncall = list_new();
      for (int i=0; i<rulelen; i++) {
	listint_remove(&stack);
	// do here something semantical
	object_t arg = list_remove(parsetree);
	if (arg) list_prepend(fncall, arg);
      }
      list_prepend(fncall, tupleobject_get(&p->tfunc, rn)); // add the function to call
      list_append(parsetree, list_to_tuple(fncall));
      oldstate = listint_last(&stack);
      state = treeintint_search(&p->jump, oldstate*1024+terminal);
      listint_append(&stack, state);  // push new state
    }
    if (p->verbose) {
      strncat(msg, "\n", buflen-strlen(msg));
      fprintf(stderr, msg);
    }
  }
	   
  // finish the lexer
  end_scanning();

  assert(list_length(parsetree) == 1);  // otherwise not completely reduced

  return list_front(parsetree);
}


string_t parser_to_string(parser_tr p)
{
  CHECK(p);
  return "for now: a parser";
}


#undef CHECK
