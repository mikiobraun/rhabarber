#include <assert.h>

#ifndef LIST_IMPLEMENTATION
struct list_s
{
  RHA_OBJECT;
  listobject_t list;
};
#endif

#ifndef CHECK
#define CHECK(x) CHECK_TYPE(list, x)
#define UNDEF_CHECK
#endif

#if defined(LIST_IMPLEMENTATION) || defined(USE_INLINES)

#endif

#ifdef UNDEF_CHECK
#undef CHECK
#endif
