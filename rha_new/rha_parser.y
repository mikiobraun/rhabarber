/* context free grammar for rhabarber */
%{
  //#define YYSTYPE any_t
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "rha_lexer.h"
#include "rha_types.h"
#include "messages.h"

#include "tuple_fn.h"
#include "list_fn.h"

extern  int yylex (void);
void yyerror (char const *);

static list_t parsetree;   /* global var -> funny name */

//static any_t wl(any_t t); // with location: adds location to parsetree

static any_t solidify(symbol_t s, list_t t);
%}

// %error-verbose

%start prog
//%expect 0         /* how many shift/reduce conflicts do we expect? */

/*
 * types declaration
 */
%union {
  any_t obj;
  list_t lis;
}

%token <obj> BOOL INT REAL STRING SYMBOL LRP RRP LSP RSP LCP RCP
%type <obj> expr
%type <lis> prog wslist

%% /* Grammar rules and actions follow.  */
prog        : /* empty */     { parsetree = list_new(); }
            | wslist          { parsetree = $1; }
            ;
wslist      : expr            { $$ = list_new(); list_append($$, $1); }    // white-spaced list
            | wslist expr     { $$ = $1; list_append($$, $2); }
            ;
expr        : BOOL            { $$=$1; }    // boolean literal
            | INT             { $$=$1; }    // integer literal
            | REAL            { $$=$1; }    // real literal
            | STRING          { $$=$1; }    // string literal
            | SYMBOL          { $$=$1; }    // symbol literal
            | LRP RRP         { $$ = solidify(rounded_sym, list_new()); }
            | LRP wslist RRP  { $$ = solidify(rounded_sym, $2); }
            | LSP RSP         { $$ = solidify(squared_sym, list_new()); }
            | LSP wslist RSP  { $$ = solidify(squared_sym, $2); }
            | LCP RCP         { $$ = solidify(curlied_sym, list_new()); }
            | LCP wslist RCP  { $$ = solidify(curlied_sym, $2); }
            ;
%%
char *currentfile;
int numerrors;

any_t solidify(symbol_t s, list_t l)
{
  list_prepend(l, WRAP_SYMBOL(s));
  return WRAP_PTR(RHA_list_t, l);
}


void yyerror (char const *s)
{
  if (currentfile)
    rha_error("in %s:%d syntax error before \'%s\' token",
	      currentfile, yylineno, yytext);
  else
    rha_error("syntax error before \'%s\' token", yytext);
  numerrors++;
}




void initparserstate(char *name)
{
  currentfile = name;
  numerrors = 0;
}

/* define functions here */

list_t rhaparsestring(char *str)
{
  parsetree = 0;
  initparserstate(NULL);
  beginstringparse(str);
  int failed = yyparse();
  if (failed) rha_error("yyparse error");
  endparse();
  if (failed || numerrors > 0)
    return 0;
  return parsetree;
}


list_t rhaparsefile(char *str)
{
  parsetree = 0;
  FILE *f = fopen(str, "r");
  if (f) {
    initparserstate(str);
    beginfileparse(f);
    int failed = yyparse();
    endparse();	
    if (failed) rha_error("parser couldn't recover");
    fclose(f);
    if (failed || numerrors > 0)
      return 0;
  }
  else {
    rha_error("could not open file \"%s\"\n", str);
  }
  return parsetree;
}
