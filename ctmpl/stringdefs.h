#ifndef STRING_DEFS_H
#define STRING_DEFS_H

#include <stdlib.h>
#include <string.h>

typedef char *string;
extern char *strdup(char *s); // we need this for C99
extern int hashstring(char *s);
extern int stringlessthan(char *s, char *t);

#endif
