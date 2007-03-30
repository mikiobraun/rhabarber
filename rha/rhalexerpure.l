/* -*- C -*- */
%{
#include <stdlib.h>
#include <string.h>
#include "messages.h"
#include "rhalexerpure.h"
#include "symbol_tr.h"
#include "int_tr.h"
#include "real_tr.h"
#include "string_tr.h"
object_t yypurelval;
%}
%option noyywrap
%%
[0-9]+			{  // literals: integer
                           yypurelval = int_new(atoi(yypuretext)); 
                           return 2; 
                        }
([0-9]+\.[0-9])|([0-9]*\.[0-9]+) { // literals: real
                           yypurelval = real_new(atof(yypuretext)); 
                           return 2; 
                        }
'[^']*' 		{  // literals: string
                           yypuretext[strlen(yypuretext) - 1] = 0;
                           yypurelval = string_new(yypuretext + 1);
                           return 2;
                        }
[a-zA-Z_][a-zA-Z0-9_]*  {  // identifier/keywords
                           yypurelval = string_new(yypuretext); 
			   return 1; 
                        }
[ \t\n]+		// eat up white space
"//".*                  // eat up comment
<<EOF>>			yyterminate();  // returns 0
.			{  // identifier/keywords: brackets, etc
                           yypurelval = string_new(yypuretext);
			   return 1;
                        }
%%
YY_BUFFER_STATE currentbufferpure;

int get_next_token() {
  return yypurelex();
}


void begin_scanning_string(const char *s)
{
  currentbufferpure = yypure_scan_string(s);
}


void end_scanning()
{
  yypure_delete_buffer(currentbufferpure);
}


void begin_scanning_file(FILE *f)
{
  currentbufferpure = yypure_create_buffer(f, YY_BUF_SIZE);
  yypure_switch_to_buffer(currentbufferpure);  
}

void thisfunctioncallsyyunputsowillyoupleaseshutupnowgccpure()
{
  yyunput(1, 0);
}
