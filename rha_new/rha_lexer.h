#ifndef RHALEXER_H
#define RHALEXER_H

#include <stdio.h>

extern char *yytext;
extern int yyleng;
extern void beginstringparse(const char *s);
extern void endparse(void);
extern void beginfileparse(FILE *);

extern int fileno(FILE *);

extern int yylineno;

#endif
