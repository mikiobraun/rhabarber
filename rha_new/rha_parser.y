/* context free grammar for rhabarber */
%{
  //#define YYSTYPE object_t
#include <stdio.h>
#include <stdbool.h>
#include "rha_lexer.h"
#include "rha_types.h"
  //#include "utils.h"
#include "messages.h"

#include "tuple_fn.h"
#include "list_fn.h"

extern  int yylex (void);
void yyerror (char const *);

static object_t parsetree;   /* global var -> funny name */

static object_t wl(object_t t); /* with location: adds location to parsetree */

static object_t solidify(symbol_t s, list_t t);
%}

// %error-verbose

%start prog
//%expect 0         /* how many shift/reduce conflicts do we expect? */

/*
 * types declaration
 */
%union {
  object_t obj;
  list_t lis;
}

%token <obj> BOOL INT REAL STRING SYMBOL LRP RRP LSP RSP LCP RCP
%type <obj> expr
%type <lis> prog wslist

%% /* Grammar rules and actions follow.  */
prog        : wslist          { solidify(curlied_sym, $1); }
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

object_t solidify(symbol_t s, list_t l)
{
  list_prepend(l, WRAP_SYMBOL(s));
  return WRAP_PTR(LIST_T, list_proto, l);
}


void yyerror (char const *s)
{
  if (currentfile)
    printf ("%s:%d: error: syntax error before \'%s\' token\n", 
	    currentfile, yylineno, yytext);
  else
    printf ("error: syntax error before \'%s\' token\n", yytext);
  numerrors++;
}




void initparserstate(char *name)
{
  currentfile = name;
  numerrors = 0;
}

/* define functions here */

object_t rhaparsestring(char *str)
{
  parsetree = 0;
  initparserstate(NULL);
  beginstringparse(str);
  int failed = yyparse();
  if (failed) printf("yyparse error\n");
  endparse();
  if (failed || numerrors > 0)
    return 0;
  return parsetree;
}


object_t rhaparsefile(char *str)
{
  parsetree = 0;
  FILE *f = fopen(str, "r");
  if (f) {
    initparserstate(str);
    beginfileparse(f);
    int failed = yyparse();
    endparse();	
    if (failed) printf("parser couldn't recover\n");
    fclose(f);
    if (failed || numerrors > 0)
      return 0;
  }
  else {
    rha_error("could not open file \"%s\"\n", str);
  }
  return parsetree;
}
