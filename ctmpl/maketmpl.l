/* -*- C -*- */
%{
#include <string.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

  int haserror;
  int wascommand;
  int writingheader;
  int writingimplementation;
  int lineno;
  int prevspace;
  FILE *out;
  extern char *prefix;
  enum { GENERATE, INFO } mode = GENERATE;

  extern int fileno(FILE *f);

  char *lookup(char *var);
  void command(char *c);
  void translate_tag(char *t);
  void translate_tag_as_type(char *t);
%}

%%
^"//@".+			{ command(yytext + 3); wascommand = 1; }
[a-zA-Z_]"<"[a-zA-Z_]+">"	if(out && mode == GENERATE && !haserror) {
                                  fprintf(out, "%c", *yytext);
                                  translate_tag(yytext + 1);
                                }
"<"[a-zA-Z_]+">"		if(out && mode == GENERATE && !haserror) {
                                  translate_tag_as_type(yytext);
                                }
"@"				if(out && mode == GENERATE && !haserror) {
                                  fprintf(out, "%s_", prefix);
                                }
.|\n				{if(out && mode == GENERATE && !haserror) {
                                  if(!wascommand)
     				    fprintf(out, "%s", yytext);
				  else
				    wascommand = 0;
				}
                                if(*yytext == '\n') lineno++;
                                }

%%
//
// Variables
// 

#define MAXVARS	64
enum inctype { ANGLES, QUOTES };

char *prefix;
int numvars;
char *vars[MAXVARS];
char *vals[MAXVARS];
int numincludes;
char *includes[MAXVARS];
enum inctype inctypes[MAXVARS];
int numfiles;
char **files;
char *currentfile;

char headerfn[1024];
char implementationfn[1024];
char headertag[1024];

//
// forward references
//

#ifndef HAVE_STRDUP
extern char *strdup(char *);
#endif
void usage(char *name);
void parse_option(char *s);
void init_arrays(int c);
void free_arrays();
void assign_variable(char *s, char *t);
void add_include(char *i, enum inctype it);
void add_file(char *f);
int begins_with(char *a, char *b);
char *find_start(char *s);
void begin_header();
void begin_implementation();
void end_either();
void expand_prefix();
void printfileline();

//
// Main programm loop
//
int main(int argc, char **argv)
{
  if(argc == 1) {
    usage(argv[0]);
  }
  else {
    init_arrays(argc - 1);

    // parse arguments
    for(int i = 1; i < argc; i++) {
      if(*argv[i] == '-')
	parse_option(argv[i] + 1);
      else if(strchr(argv[i], '=')) {
	char *p = strchr(argv[i], '=');
	*p++ = 0;
	assign_variable(argv[i], p);
      }
      else {
	add_file(argv[i]);
      }
    }

    // translate file
    for(int i = 0; i < numfiles; i++) {
      // set up variables
      haserror = 0;
      out = 0;
      writingheader = 0;
      writingimplementation = 0;
      currentfile = files[i];
      lineno = 1;

      headerfn[0] = 0;
      implementationfn[0] = 0;
      
      // try opening file
      FILE *f = fopen(files[i], "r");
      if(ferror(f)) {
	printf("Error opening \"%s\"\n", files[i]);
	break;
      }

      // set up lex and translate file
      YY_BUFFER_STATE buf = yy_create_buffer(f, 1024);
      yy_switch_to_buffer(buf);

      yylex();

      yy_delete_buffer(buf);

      end_either();

      if(out) fclose(out);
      fclose(f);

      if(haserror) {
	if(headerfn[0]) remove(headerfn);
	if(implementationfn[0]) remove(implementationfn);
	free_arrays();
	exit(EXIT_FAILURE);
      }
    }

    free_arrays();
  }
}


void usage(char *name)
{
  printf("Usage: %s [options] [file] [var1=val1 var2=val2 var3=val3 ...]\n",
	 name);
  printf("\n"
	 "options:\n"
	 "\n"
	 "        -d : show docs from template file\n"
	 "    -Iname : include <name>\n"
	 "    -iname : include \"name\"\n"
	 "        -h : show this help\n"
	 "    -oname : use prefix \"name\"\n\n");
}


void parse_option(char *s)
{
  switch(*s) {
  case 'd':
    mode = INFO;
    break;
  case 'I':
    add_include(s + 1, ANGLES);
    break;
  case 'i':
    add_include(s + 1, QUOTES);
    break;
  case 'h':
    usage("maketmpl");
    break;
  case 'o':
    prefix = strdup(s + 1);
    break;
  default:
    printf("Warning: unrecognized option '-%c'\n", *s);
  };
}


//
// The actual commands
// 
void command(char *c)
{
  static int numparams = 0;
  static int infostarted = 0;

  if(mode == INFO) {
    if(begins_with(c, "name")) {
      printf("template %s\nwith args ", find_start(c));
    }
    else if(begins_with(c, "param")) {
      if (numparams == 0)
	printf("%s", find_start(c));
      else 
	printf(", %s", find_start(c));
      numparams++;
    }
    else if(begins_with(c, "info")) {
      if(!infostarted) 
	printf("\n\n%s\n", find_start(c));
      else
	printf("%s\n", find_start(c));
      infostarted = 1;
    }
    else if(begins_with(c, "header")
	    || begins_with(c, "implementation") )
    {
      // do nothing
    }
    else {
      printfileline();
      printf("Warning: unknown command \"@%s\"\n", c);
    }
  }
  else if(mode == GENERATE) {
    if(begins_with(c, "name")) {
      if(!prefix)
	prefix = strdup(find_start(c));
    }
    else if(begins_with(c, "param")) {
      // set defaults
      char *s = find_start(c);
      char *p = strchr(s, '=');
      if(p) {
	*p++ = 0;
	if(!lookup(s))
	  assign_variable(s, p);
      }
      else {
	if(!lookup(s)) {
	  printfileline();
	  printf("Error: parameter \"%s\" has to be specified\n", s);
	  haserror = 1;
	}
      }
    }
    else if(begins_with(c, "info")) {
      // do nothing
    }
    else if(begins_with(c, "header")) {
      end_either();
      begin_header();
    }
    else if(begins_with(c, "implementation")) {
      end_either();
      begin_implementation();
    }
    else {
      printfileline();
      printf("Warning: unknown command \"@%s\"\n", c);
    }
  }
}


void translate_tag(char *s)
{
  s++;
  s[strlen(s) - 1] = 0;
  char *v = lookup(s);
  if(v) {
    int l = strlen(v);
    if(!strcmp(v + l - 2, "_t")) {  // remove "_t" at the end
      v[l-2] = 0;
      fprintf(out, "%s", v);
      v[l-2] = '_';
    }
    else if(!strcmp(v + l - 3, "_tr")) { // remove "_tr" at the end 
      v[l-3] = 0;
      fprintf(out, "%s", v);
      v[l-3] = '_';
    }
    else {
      fprintf(out, "%s", v);
    }
  }
  else {
    printf("warning: undefined variable \"%s\"\n", s);
  }
}

void translate_tag_as_type(char *s)
{
  s++;
  s[strlen(s) - 1] = 0;
  char *v = lookup(s);
  if(v)
    fprintf(out, "%s", v);
  else {
    printf("warning: undefined variable \"%s\"\n", s);
  }
}

//
// Implementation headers
//

void begin_header()
{
  expand_prefix();

  sprintf(headerfn, "%s.h", prefix); 
  out = fopen(headerfn, "w");
  if(ferror(out)) {
    printfileline();
    printf("Error: Unable to open file \"%s\"\n", headerfn);
  }		
  writingheader = 1;

  char *s = prefix;
  char *t = headertag;
  while(*s) {
    *t++ = toupper(*s++);
  }
  *t = 0;

  fprintf(out, 
	  "// Template generated by maketempl from file %s\n"
	  "//\n"
	  "// maketempl has been written by Mikio Braun, 2005\n"
	  "// mikio@first.fraunhofer.de\n"
	  "//\n"
	  "\n"
	  "#ifndef %s\n"
	  "#define %s\n\n",
	  currentfile,
	  headertag, headertag);
  
  for(int i = 0; i < numincludes; i++) {
    switch(inctypes[i]) {
    case ANGLES:
      fprintf(out, "#include <%s>\n", includes[i]);
      break;
    case QUOTES:
      fprintf(out, "#include \"%s\"\n", includes[i]);
      break;      
    }
  }
  
  fprintf(out, "\n"
	  "#line %d \"%s\"\n",
	  lineno, currentfile);
  
  writingheader = 1;
}

void end_header()
{
  fprintf(out,
	  "\n"
	  "#endif\n");
  writingheader = 0;
}

void begin_implementation()
{
  expand_prefix();

  sprintf(implementationfn, "%s.c", prefix); 
  out = fopen(implementationfn, "w");
  if(ferror(out)) {
    printfileline();
    printf("Error: Unable to open file \"%s\"\n", implementationfn);
  }		
  fprintf(out, 
	  "// Template generated by maketempl from file %s\n"
	  "//\n"
	  "// maketempl has been written by Mikio Braun, 2005\n"
	  "// mikio@first.fraunhofer.de\n"
	  "//\n"
	  "\n"
	  "#include \"%s.h\"\n"
	  "\n"
	  "\n#line %d \"%s\"\n\n",
	  currentfile,
	  prefix, lineno, currentfile);

  writingimplementation = 1;
}

void end_implementation()
{
  writingimplementation = 0;
}

void end_either()
{
  if(out) {
    if(writingheader)
      end_header();
    else if(writingimplementation)
      end_implementation();
    fflush(out);
    fclose(out);
    out = 0;
  }
}

//
// Bookkeeping
//
void init_arrays(int c)
{
  prefix = 0;
  numvars = 0;
  numincludes = 0;
  numfiles = 0;
  files = calloc(c, sizeof(char*));
}


void free_arrays()
{
  return;

  int i;
  for(i = 0; i < numvars; i++) {
    free(vars[i]);
    free(vals[i]);
  }
  for(i = 0; i < numfiles; i++) {
    free(files[i]);
  }
  free(files);
  free(prefix);
}


void assign_variable(char *s, char *t)
{
  if(numvars == MAXVARS) {
    printf("warning: cannot assign %s=%s because maximum number of variables (%d) is exceeded", s, t, MAXVARS);
    return;
  }
  vars[numvars] = strdup(s);
  vals[numvars] = strdup(t);
  numvars++;
}


void add_include(char *i, enum inctype it)
{
  includes[numincludes] = strdup(i);
  inctypes[numincludes] = it;
  numincludes++;
}


void add_file(char *n)
{
  files[numfiles++] = strdup(n);
}


char *lookup(char *p)
{
  for(int i = 0; i < numvars; i++) {
    if(!strcmp(vars[i], p)) return vals[i];
  }
  return 0;
}

//
// Misc
// 
int begins_with(char *s, char *b)
{
  return !strncmp(s, b, strlen(b));
}

char *find_start(char *s)
{
  while(*s && isalpha(*s)) s++;
  while(*s && isspace(*s)) s++;
  return s;
}

void expand_prefix()
{
  char *p, *q, *l, *s;
  
  while( (p = strchr(prefix, '<')) != 0
	 && (q = strchr(p + 1, '>')) != 0 ) {
    *q++ = 0;
    
    l = lookup(p + 1);
    if(!l) {
      *--q = '>'; // undo the cutting out of the variable name
      return; // and we're done
    }

    // else, replace prefix by version with variable name expanded
    *p++ = 0;

    int llen = strlen(l);

    s = malloc(sizeof(char) * (strlen(prefix) + strlen(q) + llen));
    s[0] = (char) 0;

    strcat(s, prefix);
    if(!strcmp(l + llen - 2, "_t")) {
      strncat(s, l, llen - 2);
    }
    else if(!strcmp(l + llen - 3, "_tr")) {
      strncat(s, l, llen - 3);
    }
    else {
      strcat(s, l);
    }
    strcat(s, q);

    free(prefix);
    prefix = s;
  }
  //printf("expanded prefix is: %s\n", prefix);
}


//
// error messages
// 

void printfileline()
{
  printf("%s:%d: ", currentfile, lineno);
}

