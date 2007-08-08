/* context free grammar for rhabarber */
%{
  //#define YYSTYPE object_t
#include <stdio.h>
#include <stdbool.h>
#include "rha_lexer.h"
#include "rha_types.h"
#include "utils.h"
#include "messages.h"

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

/* returns list_tr */
semiclist : expr                      { $$ = list_new(); list_append($$, $1); $$ = wl($$); }
          | semiclist SEMIC expr      { list_append($1, $3); $$ = wl($1); }
          | semiclist SEMIC           { $$ = wl($1); }
          ;


/* expression, returns object_t, does evaluate to some value */
expr      : wslist                    { $$ = wl(list_to_tuple($1)); }
          ;

/* white_space_list, returns list_tr */
wslist    : term                      { $$ = list_new();
                                        list_append($$, $1); $$ = wl($$); }
          | wslist term               { list_append($1, $2); $$ = wl($1); }
          ;

/* terms, returns object_t */
term      : INT                       { $$ = wl($1); }
          | BOOL                      { $$ = wl($1); }
	  | REAL                      { $$ = wl($1); }
          | STRING                    { $$ = wl($1); }
          | SYMBOL                    { $$ = wl($1); }
// grouped expressions, e.g. (17+42), note that we need to remember
// the grouping until the macros are resolved, because grouped
// expressions can also be singleton argument lists
          | LRP expr RRP              { $$ = wl(tuple_make(2, tuplefy_sym, $2)); }
// non-singleton tuples, e.g. (), (17, 42)
          | LRP RRP                   { $$ = wl(tuple_make(1, tuplefy_sym)); }     // empty tuples, e.g. ()
          | LRP commalist RRP         { list_prepend($2, tuplefy_sym); 
                                        $$ = wl(list_to_tuple($2)); }          // tuples with more than one element, e.g. (7, 4)
// complicated literals, e.g. [ ], [17, 42; 13, 57]
| LSP RSP                   { $$ = wl(tuple_make(1, literal_sym)); } // empty literals, e.g. [ ]
          | LSP litlist RSP           { list_prepend($2, literal_sym);
                                        $$ = list_to_tuple($2); } // literals e.g. [17; 42]
// code blocks, e.g. { }, {bla; blub }
          | LCP RCP                   { $$ = wl(tuple_make(1, do_sym)); }  // empty code block, e.g. { }
	  | LCP semiclist RCP         { list_prepend($2, do_sym);
	                                $$ = list_to_tuple($2); }          // some non-empty code block, e.g. { bla; blub }
/* finally some error-generating stuff */
          | error
          ;

/* comma-separated list of expression, returns list_tr */
commalist : expr COMMA expr           { $$ = list_new(); list_append($$, $1); list_append($$, $3); $$ = wl($$); }
          | commalist COMMA expr      { list_append($1, $3); $$ = wl($1); }
          ;

/* separator for programmable literals, returns object_t */
litsep    : COLON                   { $$ = wl(string_new("COLON")); }
          | COMMA                   { $$ = wl(string_new("COMMA")); }
          | SEMIC                   { $$ = wl(string_new("SEMIC")); }
          | ENTAILS                 { $$ = wl(string_new("ENTAILS")); }
          ;

/* literal generating list, returns list_tr */
litlist   : expr                    { $$ = list_new(); list_append($$, $1); $$ = wl($$); }
          | litlist litsep expr     { list_append($1, $2); list_append($1, $3); $$ = wl($1); }
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
