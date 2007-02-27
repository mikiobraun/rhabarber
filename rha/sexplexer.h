#ifndef SEXPLEXER_H
#define SEXPLEXER_H

#include <stdio.h>

enum tokens 
{ 
  END = 0, LPAREN, RPAREN, SYMBOL, STRING, INTEGER, REAL
};

extern int nexttoken();
extern char *yytext;
extern int yyleng;
extern void beginstringparse(const char *s);
extern void endparse();

extern int fileno(FILE *);

#endif
