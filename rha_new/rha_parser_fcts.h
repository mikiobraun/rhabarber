#ifndef RHA_PARSER_H
#define RHA_PARSER_H

#include <stdio.h>
#include "list_fn.h"

extern bool parseprintresult;
extern list_t rhaparsestring(const char *str);
extern list_t rhaparsefile(const char *str);

#endif
