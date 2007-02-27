#define SYMBOL_IMPLEMENTATION
#include "symbol_tr.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "alloc.h"

#include "object.h"
#include "primtype.h"
#include "string_tr.h"

#define CHECK(s) CHECK_TYPE(symbol, s)

primtype_t symbol_type = 0;

#ifdef USE_INLINES
#define INLINE inline
#else
#define INLINE
#endif
#include "symbol_inline.c"

#ifdef USE_STRINGS


void symbol_init(void)
{
  if (!symbol_type) {
    symbol_type = primtype_new("symbol", sizeof(struct symbol_s));
    (void) symbol_new("");
  }
}


symbol_tr symbol_new(string_t text)
{
  symbol_tr s = object_new(symbol_type);
  s->name = gc_strdup(text);
  return s;
}


int symbol_hash(symbol_tr s)
{
  CHECK(s);
  int hash = 0;
  for(const char *p = symbol_name(s); *p; p++) {
    hash += *p;
  }
  return hash;
}



#elif defined(USE_UNIQUEID)

#include "treestringint.h"
#include "treeintstring.h"

static int highestid = 0;
static treestringint_t symbols;
static treeintstring_t symbolnames;

void symbol_init(void)
{
  if (!symbol_type) {
    symbol_type = primtype_new("symbol", sizeof(struct symbol_s));

    // initialize symbols tree
    treestringint_init(&symbols);
    treeintstring_init(&symbolnames);

    // create protypical element
    (void) symbol_new("");
  }
}

symbol_tr symbol_new(string_t text)
{
  symbol_tr s = object_new(symbol_type);
  
  int id = treestringint_search(&symbols, (char*) text);
  if(!id) { // if not found, insert into symbols and set id
    ++highestid;
    treestringint_insert(&symbols, gc_strdup(text), highestid);
    id = highestid;
    treeintstring_insert(&symbolnames, highestid, gc_strdup(text));
  }
    
  s->symid = id; 
  return s;  
}

string_t symbol_name(symbol_tr s)
{
  CHECK(s);
  return treeintstring_search(&symbolnames, s->symid);
}

#else
#  error "Unknown implementation requested for symbol.c"
#endif

string_t symbol_to_string(symbol_tr s)
{
  CHECK(s);
  return symbol_name(s);
}
