#ifndef RHAPARSER_H
#define RHAPARSER_H

#include <stdio.h>
#include "list_tr.h"

extern bool parseprintresult;
extern list_tr rhaparsestring(const char *str);
extern list_tr rhaparsefile(const char *str);

#endif
