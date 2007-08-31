#include <assert.h>
#include "symbol_fn.h"
#include "gtree.h"
#include "alloc.h"

static int symbolcount = 0;
static struct gtree symbolids;
static struct gtree symbolnames;

static bool symbol_lessfct(intptr_t a, intptr_t b)
{
  return strcmp((void*) a, (void*) b) < 0;
}

static bool symbol_namelessfct(intptr_t a, intptr_t b)
{
  return a < b;
}

symbol_t symbol_new(string_t s)
{
  if (!symbolcount) {
    gtree_init(&symbolids, symbol_lessfct);
    gtree_init(&symbolnames, symbol_namelessfct);
  }

  int i = gtree_searchi(&symbolids, s );
  if (!i) {
    symbolcount++;
    i = symbolcount;
    s = gc_strdup(s);
    gtree_insert(&symbolids, s, i);
    gtree_insert(&symbolnames, i, s);
  }
  return i;
}

bool_t symbol_equal(symbol_t s, symbol_t t)
{
  return s == t;
}

string_t symbol_name(symbol_t s)
{
  // catch the case that s==0, which should only happen if we access
  // 'symbol.proto'
  if (!s) return "<no symbolname for zero>";
  string_t n = gtree_searchp(&symbolnames, s);
  assert(n);
  return n;
}

string_t symbol_to_string(symbol_t s)
{
  return symbol_name(s);
}
