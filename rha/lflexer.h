#ifndef LFLEXER_H
#define LFLEXER_H

#include <stdio.h>

extern char *yytext;
extern void beginstringparse(const char *s);
extern void endparse();

extern int fileno(FILE *);

extern char *leftparens[];
extern char *rightparens[];
extern char *opnames[];

#endif
