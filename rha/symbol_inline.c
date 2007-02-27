#include <assert.h>
#include "alloc.h"
/*
 * Symbols inline part
 */
#ifndef CHECK
#define CHECK(s) CHECK_TYPE(symbol, s)
#define UNDEF_CHECK
#endif

#ifdef USE_STRINGS

struct symbol_s {
  RHA_OBJECT;
  char *name;
};

#if defined(SYMBOL_IMPLEMENTATION) || defined(USE_INLINES)
INLINE
symbol_tr symbol_clone(symbol_tr s)
{
  CHECK(s);
  return symbol_new(symbol_name(s));
}

INLINE
bool symbol_less_symbol(symbol_tr s1, symbol_tr s2)
{
  CHECK(s1);
  CHECK(s2);
  return strcmp(symbol_name(s1), symbol_name(s2)) < 0;
}

INLINE
bool symbol_greater_symbol(symbol_tr s1, symbol_tr s2)
{
  CHECK(s1);
  CHECK(s2);
  return strcmp(symbol_name(s1), symbol_name(s2)) > 0;
}

INLINE
bool symbol_equal_symbol(symbol_tr s1, symbol_tr s2)
{
  CHECK(s1);
  CHECK(s2);
  return !strcmp(symbol_name(s1), symbol_name(s2));
}

INLINE
const char *symbol_name(symbol_tr s)
{
  CHECK(s);
  return s->name;
}
#endif

#elif defined(USE_UNIQUEID)

struct symbol_s {
  RHA_OBJECT;
  int symid;
};

#if defined(SYMBOL_IMPLEMENTATION) || defined(USE_INLINES)
INLINE
symbol_tr symbol_clone(symbol_tr s)
{
  symbol_tr sc = object_new(symbol_type);
  sc->symid = s->symid;
  return sc;
}

INLINE
bool symbol_less_symbol(symbol_tr s1, symbol_tr s2)
{
  CHECK(s1);
  CHECK(s2);
  return s1->symid < s2->symid;
}

INLINE
bool symbol_greater_symbol(symbol_tr s1, symbol_tr s2)
{
  CHECK(s1);
  CHECK(s2);
  return s1->symid > s2->symid;
}

INLINE
bool symbol_equal_symbol(symbol_tr s1, symbol_tr s2)
{
  CHECK(s1);
  CHECK(s2);
  return s1->symid == s2->symid;
}

INLINE
int symbol_hash(symbol_tr s)
{
  return s->symid;
}
#endif


#else
#ifndef MAKE_DEPEND
#  error "Unknown implementation requested for symbol.c"
#endif
#endif

#ifdef UNDEF_CHECK
#undef CHECK
#endif

/*
  log:

  mikio: rewrote functions such that only symbol_name() accesses s->name.
*/
