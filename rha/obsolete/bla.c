#include <stdio.h>
#include "object.h"
#include "list.h"
#include "alloc.h"
#include "symtable.h"
#include "dispatch.h"
#include "builtin.h"
#include "literals.h"

object_t Integer(int i);

object_t printSymbol(list_t in)
{
  symbol_t s = CAST(symbol, getdata(listfront(in)));

  printf("Symbol: %s\n", getstringfromsymbol(s));
  return 0;
}

object_t printInteger(list_t in)
{
  intvalue_t i = CAST(intvalue, getdata(listfront(in)));

  printf("%d", getintvalue(i));
  return 0;
}

object_t integerAdd(list_t in)
{
  listiter_t li;
  initlistiter(li, in);
  intvalue_t i1 = CAST(intvalue, getdata(li_get(li))); li_next(li);
  intvalue_t i2 = CAST(intvalue, getdata(li_get(li)));
  
  return Integer(i1->value + i2->value);
}

object_t Integer(int i)
{
  object_t o = MAKEOBJECT(intvalue, newintvalue(i));
  assign(o, newsymbol("print"), MAKEOBJECT(builtin, newbuiltin(printInteger)));
  assign(o, newsymbol("+"), MAKEOBJECT(builtin, newbuiltin(integerAdd)));  

  return o;
}

int main()
{
  GC_init();

  object_t o = MAKEOBJECT(symbol, newsymbol("foo"));
  assign(o, newsymbol("print"), MAKEOBJECT(builtin, newbuiltin(printSymbol)));

  dumpobject(o);

  object_t i1 = Integer(42);
  object_t i2 = Integer(7);

  object_t i3 = dispatch1(i1, newsymbol("+"), i2);
  dispatch0(i3, newsymbol("print"));
  printf("\n");
}
