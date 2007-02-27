%{
YY_BUFFER_STATE	stringbuf;
%}

ALPHA		[a-zA-Z_]
DIGIT		[0-9]
%%
{ALPHA}+		return 1;
{DIGIT}+		return 1;
{DIGIT}+"."{DIGIT}* 	return 1;
{DIGIT}*"."{DIGIT}+ 	return 1;
[+/\-*!<>|=;:@]		return 1;
[\(\)\[\]\{\}]		return 1;
[ \t\n]+		/* eat up whitespace */
==|\+\+			return 1;
<<EOF>>			return 0;
.			printf("Unrecognized character: %s\n", yytext);
%%
void beginscanstring(const char *str) 
{
  stringbuf = yy_scan_string(str);
}

void endscanstring()
{
  yy_delete_buffer(stringbuf);
}
