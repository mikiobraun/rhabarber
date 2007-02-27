/* context free grammar for rhabarber */
%{
#define YYSTYPE object_t
#include <stdio.h>
#include <stdbool.h>
#include "rhalexer.h"
#include "object.h"
#include "core.h"
#include "none_tr.h"
#include "tuple_tr.h"
#include "int_tr.h"
#include "list_tr.h"
#include "string_tr.h"
#include "utils.h"
#include "messages.h"
#include "none_tr.h"

extern  int yylex (void);
void yyerror (char const *);

static object_t parsetree;   /* global var -> funny name */

static object_t wl(object_t t); /* with location: adds location to parsetree */
%}

// %error-verbose

%start prog
//%expect 0         /* how many shift/reduce conflicts do we expect? */

%token LRP RRP
%token SYMBOL STRING INT REAL BOOL
%token FN
%token THIS THAT PARENT VOID

%nonassoc BS
%left WSL
%nonassoc LSP RSP
%left SEMIC
%right IMPORT
%nonassoc TRY CATCH
%nonassoc WHILE FOR
%nonassoc IF
%nonassoc ELSE
%nonassoc FN
%right RETURN
%right BREAK
%right DELIVER
%right THROW
%nonassoc ENTAILS
%left COMMA
%right ASSIGN PLUSASSIGN MINUSASSIGN TIMESASSIGN DIVIDEASSIGN SNATCH
%right SYMASSIGN
%nonassoc COLON
%left MAPS
%left NOT
%left AND OR
%left LESS LESSEQUAL GREATER GREATEREQUAL EQUAL NOTEQUAL
%left PLUS MINUS
%left TIMES DIVIDE
%left PLUSPLUS MINUSMINUS
%right PRE  // for ++x, --x
%right NEG
%nonassoc LRP RRP
%left DOT QUEST NOBINDDOT
%nonassoc LCP RCP 

%% /* Grammar rules and actions follow.  */
prog      : semiclist                 { parsetree = $1; }
          | /* empty */               { parsetree =  0; }
          ;

/* terms, returns object_t */
term      : pexpr                     { $$ = wl($1); }  // -1, --1, ++1, -1+2, -x
          | mexpr                     { $$ = wl($1); }  // if (true) 17 else 42, while f(x) g(x)
          | BS term                   { $$ = wl(tuple_make(2, quote_sym, $2)); }
          ;

nexpr     : aexpr                     { $$ = wl($1); }  // x, x+y
          | lexpr                     { $$ = wl($1); }  // [17, 42]
          | texpr                     { $$ = wl($1); }  // (), (x,), (x, y)
          ; 

// * the distinction between expr and mexpr has the effect that operators like + (see pexpr and aexpr) binds stronger than macro stuff.
// * gexpr and nexpr are equally handled in expr but not in mexpr (macros), there we need to mark gexpr with group_sym
expr      : nexpr                     { $$ = wl($1); }
          | gexpr                     { $$ = wl($1); }  // (x+y), (x)
          ;

/* pexpr is for stuff like -17, ++17, --17, note that 1 * -1 is not allowed, use 1 * (-1) */
pexpr     : MINUS expr %prec NEG      { $$ = wl(tuple_make(3, minus_sym, int_new(0), $2)); }
          | PLUSPLUS expr %prec NEG   { $$ = wl(tuple_make(2, plusplus_sym, tuple_make(2, quote_sym, $2))); }
          | MINUSMINUS expr %prec NEG { $$ = wl(tuple_make(2, minusminus_sym, tuple_make(2, quote_sym, $2))); }
/* infix operators relevant for prefixed expressions: */ 
	  | pexpr PLUS expr           { $$ = wl(tuple_make(3, plus_sym, $1, $3)); }
	  | pexpr MINUS expr          { $$ = wl(tuple_make(3, minus_sym, $1, $3)); }
          | pexpr TIMES expr          { $$ = wl(tuple_make(3, times_sym, $1, $3)); }
	  | pexpr DIVIDE expr         { $$ = wl(tuple_make(3, divide_sym, $1, $3)); }
          | pexpr LESS expr           { $$ = wl(tuple_make(3, less_sym, $1, $3)); }
          | pexpr LESSEQUAL expr      { $$ = wl(tuple_make(3, lessequal_sym, $1, $3)); }
	  | pexpr GREATER expr        { $$ = wl(tuple_make(3, greater_sym, $1, $3)); }
	  | pexpr GREATEREQUAL expr   { $$ = wl(tuple_make(3, greaterequal_sym, $1, $3)); }
	  | pexpr EQUAL expr          { $$ = wl(tuple_make(3, equal_sym, $1, $3)); }
	  | pexpr NOTEQUAL expr       { $$ = wl(tuple_make(3, notequal_sym, $1, $3)); }
          ;

/* arithmetic expressions, return object_t */
aexpr      
/* simple literals: */
          : INT                     { $$ = wl($1); }
          | BOOL                    { $$ = wl($1); }
	  | REAL                    { $$ = wl($1); }
          | STRING                  { $$ = wl($1); }
          | SYMBOL                  { $$ = wl($1); } 
/* praefix operators: */
/*           | BS expr                 { $$ = wl(tuple_make(2, quote_sym, $2)); } */
          | NOT expr                { $$ = wl(tuple_make(2, not_sym, $2)); }
/* postfix operators */
          | expr PLUSPLUS           { $$ = wl(tuple_make(2, postplusplus_sym, tuple_make(2, quote_sym, $1))); }
          | expr MINUSMINUS         { $$ = wl(tuple_make(2, postminusminus_sym, tuple_make(2, quote_sym, $1))); }
/* infix operators: */ 
          | expr SNATCH expr        { $$ = wl(tuple_make(3, snatch_sym, $1, $3)); }
          | expr PLUSASSIGN expr    { $$ = wl(tuple_make(3, assign_sym, $1, tuple_make(3, plus_sym, $1, $3))); }
          | expr MINUSASSIGN expr   { $$ = wl(tuple_make(3, assign_sym, $1, tuple_make(3, minus_sym, $1, $3))); }
          | expr TIMESASSIGN expr   { $$ = wl(tuple_make(3, assign_sym, $1, tuple_make(3, times_sym, $1, $3))); }
          | expr DIVIDEASSIGN expr  { $$ = wl(tuple_make(3, assign_sym, $1, tuple_make(3, divide_sym, $1, $3))); }
	  | expr PLUS expr          { $$ = wl(tuple_make(3, plus_sym, $1, $3)); }
	  | expr MINUS expr         { $$ = wl(tuple_make(3, minus_sym, $1, $3)); }
          | expr TIMES expr         { $$ = wl(tuple_make(3, times_sym, $1, $3)); }
	  | expr DIVIDE expr        { $$ = wl(tuple_make(3, divide_sym, $1, $3)); }
          | expr LESS expr          { $$ = wl(tuple_make(3, less_sym, $1, $3)); }
          | expr LESSEQUAL expr     { $$ = wl(tuple_make(3, lessequal_sym, $1, $3)); }
	  | expr GREATER expr       { $$ = wl(tuple_make(3, greater_sym, $1, $3)); }
	  | expr GREATEREQUAL expr  { $$ = wl(tuple_make(3, greaterequal_sym, $1, $3)); }
	  | expr EQUAL expr         { $$ = wl(tuple_make(3, equal_sym, $1, $3)); }
	  | expr NOTEQUAL expr      { $$ = wl(tuple_make(3, notequal_sym, $1, $3)); }

	  | expr DOT SYMBOL         { $$ = wl(tuple_make(3, dot_sym, $1, $3)); }
	  | expr NOBINDDOT SYMBOL   { $$ = wl(tuple_make(3, nobinddot_sym, $1, $3)); }
	  | expr QUEST SYMBOL       { $$ = wl(tuple_make(3, quest_sym, $1, $3)); } 

	  | expr DOT LRP expr RRP   { $$ = wl(tuple_make(3, dot_sym, $1, tuple_make(2, id_sym, $4))); }
	  | expr NOBINDDOT LRP expr RRP { $$ = wl(tuple_make(3, nobinddot_sym, $1, tuple_make(2, id_sym, $4))); }
	  | expr QUEST LRP expr RRP { $$ = wl(tuple_make(3, quest_sym, $1, tuple_make(2, id_sym, $4))); } 

	  | expr AND expr           { $$ = wl(tuple_make(3, and_sym, $1, $3)); }
	  | expr OR expr            { $$ = wl(tuple_make(3, or_sym, $1, $3)); }
/* assignment */
          | expr ASSIGN expr        { $$ = wl(tuple_make(3, assign_sym, $1, $3)); }
/* blocks */
          | LCP RCP                 { $$ = wl(tuple_make(1, do_sym)); }
	  | LCP semiclist RCP       { list_prepend($2, do_sym); 
	                              $$ = list_solidify($2);
	                              for (int i=1; i<tuple_length($$); i++ )
					tuple_set($$, i, tuple_make(2, quote_sym, tuple_get($$, i))); 
	                              $$ = wl($$); }
/* finally some error-generating stuff */
          | error
 	  ;

/* tuple, returns object_t */
texpr     : LRP RRP                 { $$ = tuple_make(1, tuplefy_sym); }
          | LRP term COMMA RRP      { $$ = tuple_make(2, tuplefy_sym, $2); }
          | LRP commalist RRP       { list_prepend($2, tuplefy_sym); $$ = wl(list_solidify($2)); }
          ;

/* returns list_tr */
commalist : term COMMA term         { $$ = list_new(); list_append($$, $1); list_append($$, $3); $$ = wl($$); }
          | commalist COMMA term    { list_append($1, $3); $$ = wl($1); }
          ;

/* returns list_tr */
semiclist : term                    { $$ = list_new(); list_append($$, $1); $$ = wl($$); }
          | semiclist SEMIC term    { list_append($1, $3); $$ = wl($1); }
          | semiclist SEMIC         { $$ = wl($1); }
          ;

/* grouping expression */
gexpr     : LRP term RRP            { $$ = wl($2); }
          ;

/* macro_expression, returns object_t */
mexpr     : mlist                   { if (list_length($1) == 1) $$ = wl(list_front($1));
                                      else $$ = wl(list_solidify($1)); }
          ;

/* white_space_list, returns list_tr */
mlist     : nexpr                   { $$ = list_new(); list_append($$, $1); $$ = wl($$); }
          | mlist nexpr             { list_append($1, $2); $$ = wl($1); }
          | gexpr                   { $$ = list_new(); list_append($$, tuple_make(2, group_sym, $1)); $$ = wl($$); }
          | mlist gexpr             { list_append($1, tuple_make(2, group_sym , $2)); $$ = wl($1); }
          ;

/* programmable literals, returns object_t */
lexpr     : LSP RSP                 { $$ = wl(tuple_make(2, literal_sym, none_obj)); }
	  | LSP litlist RSP         { $2 = list_solidify($2);
				      $$ = wl(tuple_make(2, literal_sym, tuple_make(2, quote_sym, $2))); }
          ;

/* separator for programmable literals, returns object_t */
litsep    : COLON                   { $$ = wl(string_new("COLON")); }
          | COMMA                   { $$ = wl(string_new("COMMA")); }
          | SEMIC                   { $$ = wl(string_new("SEMIC")); }
          | ENTAILS                 { $$ = wl(string_new("ENTAILS")); }
          ;

/* literal generating list, returns list_tr */
litlist   : term                    { $$ = list_new(); list_append($$, $1); $$ = wl($$); }
          | litlist litsep term     { list_append($1, $2); list_append($1, $3); $$ = wl($1); }
          ;

%%
char *currentfile;
int numerrors;
string_tr currentlocation = 0;

void yyerror (char const *s)
{
  if (currentfile)
    printf ("%s:%d: error: syntax error before \'%s\' token\n", 
	    currentfile, yylineno, yytext);
  else
    printf ("error: syntax error before \'%s\' token\n", yytext);
  numerrors++;
}


string_tr getloc()
{
  static int savedlineno = 0;
  if (!currentlocation || yylineno != savedlineno) {
    if (currentfile != NULL) {
      currentlocation = string_new(sprint("%s:%d", currentfile, yylineno));
      savedlineno = yylineno;
    }
  }
  return currentlocation;
}


object_t wl(object_t o)
{
  object_t loc = getloc();
  if(loc) 
    object_assign(o, rha_location_sym, loc);
  return o;
}


void initparserstate(char *name)
{
  currentfile = name;
  numerrors = 0;
  currentlocation = 0;
}

/* define functions here */

list_tr rhaparsestring(char *str)
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


list_tr rhaparsefile(char *str)
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
