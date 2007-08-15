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
extern int yylineno;
%}
%option noyywrap
%%
"//".*           /* eat up comment (THE ORDER OF THE RULES MATTERS!!!) */
[ \t]+           /* eat up white spaces */
"false"          { yylval.obj = WRAP_BOOL(false);    return BOOL; }
"true"           { yylval.obj = WRAP_BOOL(true);     return BOOL; }

"("		 return LRP;     /* round paren */
")"		 return RRP;

"{"		 return LCP;     /* curly paren */
"}"		 return RCP;

"["		 return LSP;     /* squared paren */
"]"		 return RSP;

[a-zA-Z_][a-zA-Z0-9_]* { 
                   yylval.obj = WRAP_SYMBOL(symbol_new(yytext));
                   return SYMBOL; 
                 }
\"[^\"]*\"	 { 
                   yytext[strlen(yytext) - 1] = 0;
                   yylval.obj = WRAP_PTR(STRING_T, string_proto, escexpand(yytext + 1));
                   return STRING;
                 }
[0-9]+		 { yylval.obj = WRAP_INT(atoi(yytext)); return INT; }
([0-9]+\.[0-9])|([0-9]*\.[0-9]+) { 
                   yylval.obj = WRAP_REAL(atof(yytext)); 
		   return REAL; 
                 }
([+\-*/<>^!=.]+|"&&"|"||"|"\\"|"?"|"#"|";"|",") {
                   yylval.obj = WRAP_SYMBOL(symbol_new(yytext)); 
		   return SYMBOL; 
                 }
\n[ \t]*\n       { yylval.obj = WRAP_SYMBOL(symbol_new(";")); return SYMBOL; }
\n		 yylineno++;
.		 printf("Warning: Unrecognized character \"%s\"\n", yytext);
<<EOF>>		 yyterminate();
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
