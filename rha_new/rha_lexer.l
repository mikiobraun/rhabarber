/* -*- C -*- */
%{
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "rha_types.h"
#include "object.h"
#include "symbol_fn.h"
#include "alloc.h"

  //#include "rhalexer.h"
  //#include "rhaparser.h"
#include "rha_parser.tab.h"

extern int fileno(FILE*);
char *escexpand(char *);
%}

%%
"false"                 { yylval = WRAP_BOOL(false);    return BOOL; }
"true"                  { yylval = WRAP_BOOL(true);     return BOOL; }

"("			return LRP;     /* round paren */
")"			return RRP;

"{"			return LCP;     /* curly paren */
"}"			return RCP;

"["			return LSP;     /* squared paren */
"]"			return RSP;

[a-zA-Z_][a-zA-Z0-9_]* { yylval = WRAP_SYMBOL(symbol_new(yytext)); return SYMBOL; }
\"[^\"]*\"		{ 
                           yytext[strlen(yytext) - 1] = 0;
                           yylval = WRAP_PTR(STRING_T, string_proto, escexpand(yytext + 1));
                           return STRING;
                        }
[0-9]+			{ yylval = WRAP_INT(atoi(yytext)); return INT; }
([0-9]+\.[0-9])|([0-9]*\.[0-9]+) { yylval = WRAP_REAL(atof(yytext)); return REAL; }
([+\-*/<>^!=.]+|"&&"|"||"|"\\"|"?"|"#") {
                          yylval = WRAP_SYMBOL(symbol_new(yytext)); return SYMBOL; 
                        }
[ \t]+			/* eat up white spaces */
\n			yylineno++;
"//".*                  /* eat up comment */
.			printf("Warning: Unrecognized character \"%s\"\n", yytext);
<<EOF>>			yyterminate();
%%
YY_BUFFER_STATE currentbuffer;
int yylineno;


void beginstringparse(const char *s)
{
  currentbuffer = yy_scan_string(s);
  yylineno = 1;
}


void endparse()
{
  yy_delete_buffer(currentbuffer);
}


void beginfileparse(FILE *f)
{
  currentbuffer = yy_create_buffer(f, YY_BUF_SIZE);
  yy_switch_to_buffer(currentbuffer);  
  yylineno = 1;
}

void thisfunctioncallsyyunputsowillyoupleaseshutupnowgcc()
{
  yyunput(1, 0);
}

char *escexpand(char *in)
{
  int l = strlen(in);
  char *out = ALLOC_RAW(l+1);
  char *p = out;

  while(*in) {
    if (*in == '\\') {
      switch(*++in) {
      case 'n': *p++ = '\n'; break;
      case 't': *p++ = '\t'; break;
      default: *p++ = '\\';
	*p++ = *in;
      }
      in++;
    }
    else
      *p++ = *in++;
  }
  *p++ = '\0';
  return out;
}
