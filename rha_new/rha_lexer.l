/* -*- C -*- */
%{
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "rha_types.h"
  //#include "rhalexer.h"
  //#include "rhaparser.h"
#define YYSTYPE object_t /* is this really correct? */
#include "rha_parser.tab.h"

char *escexpand(char *);
%}

%%
("op"([+*-/<>!]|"<="|">="|"&&"|"||"|"!="|"=="|"\\"|"\."|"?")) {
                          yylval = symbol_new(yytext); return SYMBOL; 
                        }
"("			return LRP;     /* round paren */
")"			return RRP;
"{"			return LCP;     /* curly paren */
"}"			return RCP;
"["			return LSP;     /* squared paren */
"]"			return RSP;
","                     return COMMA;
";"                     return SEMIC;
":"                     return COLON;
":-"                    return ENTAILS;
\\			{ yylval = quote_sym;          return SYMBOL; } /* single back slash */
"."                     { yylval = dot_sym;            return SYMBOL; }
"?"                     { yylval = quest_sym;          return SYMBOL; }
"="                     { yylval = assign_sym;         return SYMBOL; }
"+="                    { yylval = plusassign_sym;     return SYMBOL; }
"-="                    { yylval = minusassign_sym;    return SYMBOL; }
"*="                    { yylval = timesassign_sym;    return SYMBOL; }
"/="                    { yylval = divideassign_sym;   return SYMBOL; }
"=?"                    { yylval = snatch_sym;         return SYMBOL; }
"+"                     { yylval = plus_sym;           return SYMBOL; }
"-"                     { yylval = minus_sym;          return SYMBOL; }
"*"                     { yylval = times_sym;          return SYMBOL; }
"/"                     { yylval = divide_sym;         return SYMBOL; }
"<"                     { yylval = less_sym;           return SYMBOL; }
"<="                    { yylval = lessequal_sym;      return SYMBOL; }
">"                     { yylval = greater_sym;        return SYMBOL; }
">="                    { yylval = greaterequal_sym;   return SYMBOL; }
"=="                    { yylval = equal_sym;          return SYMBOL; }
"!="                    { yylval = notequal_sym;       return SYMBOL; }
"++"                    { yylval = plusplus_sym;       return SYMBOL; }
"--"                    { yylval = minusminus_sym;     return SYMBOL; }
"&&"                    { yylval = and_sym;            return SYMBOL; }
"||"                    { yylval = or_sym;             return SYMBOL; }
"!"                     { yylval = not_sym;            return SYMBOL; }
"false"                 { yylval = bool_new(false);    return BOOL; }
"true"                  { yylval = bool_new(true);     return BOOL; }
[@#a-zA-Z_][a-zA-Z0-9_]* { yylval = symbol_new(yytext); return SYMBOL; }
\"[^\"]*\"		{ 
                           yytext[strlen(yytext) - 1] = 0;
                           yylval = string_new(escexpand(yytext + 1));
                           return STRING;
                        }
[0-9]+			{ yylval = int_new(atoi(yytext)); return INT; }
([0-9]+\.[0-9])|([0-9]*\.[0-9]+) { yylval = real_new(atof(yytext)); return REAL; }
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
