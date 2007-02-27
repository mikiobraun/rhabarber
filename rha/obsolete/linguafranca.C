#include <list>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>
#include <map>
#include <typeinfo>

using std::list;
using std::string;
using std::map;

extern void beginscanstring(const char *str);
extern void endscanstring();
extern int yylex();
extern char *yytext;

void sayhello();
void saybye();
void parseline(char *line);
void initparser();

int main()
{
  using_history();
  sayhello();
  initparser();

  char prompt[100];
  int lineno = 1;

  while(1) {
    // read line
    sprintf(prompt, "[%d]> ", lineno);
    char *line = readline(prompt);
    if (!line) break;
    add_history(line);

    // Process line
    //printf("You said: %s\n", line);
    parseline(line);

    // cleanup & next round
    free(line);
    lineno++;
  }

  saybye();
}

void sayhello()
{
  printf("RHA: LINGUA FRANCA PARSER\n"
	 "--RHA is the brainchild of Stefan Harmeling and Mikio Braun\n"
	 "--This parser was written by Mikio Braun, 2005\n");
}

void saybye()
{
  printf("\nBye.\n");
}

//////////////////////////////////////////////////////////////////////
// The parser
//////////////////////////////////////////////////////////////////////

// Data types
struct pt 
{
  virtual void print() = 0;
  virtual ~pt() {};
};

typedef list<pt*> exprlist;
typedef list<pt*>::iterator exprlistiter;

struct token : public pt
{
  string t;
  token(const char *t_) : t(t_) {};
  void print() { printf("%s", t.c_str()); };
};

struct expr : public pt
{
  exprlist l;
  ~expr() {
    for(exprlistiter it = l.begin(); it != l.end(); it++)
      delete *it;
  };

  void print() { 
    printf("(");
    exprlistiter it = l.begin();
    if(it != l.end()) {
      (*it)->print();
    }
    it++;
    for(; it != l.end(); it++) {
      printf(" ");
      (*it)->print();
    }
    printf(")");
  };
};

// setting up the parse
map<string,int> optable;
map<string,const char*> parentable;

void initparser()
{
  optable["@"] = 0;
  optable["*"] = 1;
  optable["/"] = 1;
  optable["+"] = 2;
  optable["-"] = 2;
  optable[":"] = 3;
  optable["<"] = 9;
  optable[">"] = 9;
  optable["="] = 10;
  optable["=="] = 10;

  parentable[")"] = "paren";
  parentable["}"] = "codeblock";
}
const int MAXPRED = 100;


int parseops(exprlist &tokens)
{
  // find lowest op operator
  int op = MAXPRED;
  exprlistiter opit = tokens.end();
  for(exprlistiter it = tokens.begin();
      it != tokens.end();
      it++) {
    token *t = dynamic_cast<token*>(*it);
    if(t) {
      map<string,int>::iterator i = optable.find(t->t);
      if(i != optable.end()
	 && i->second < op) {
	opit = it;
	op = i->second;
      }
    }
  }

  // if we have one, apply the associated change
  if (opit != tokens.end()) {
    exprlistiter leftit = opit; leftit--;
    exprlistiter rightit = opit; rightit++;
    exprlistiter afterit = rightit; afterit++;
    
    expr *subexpr = new expr;
    subexpr->l.push_back(*opit);
    subexpr->l.push_back(*leftit);
    subexpr->l.push_back(*rightit);

    tokens.erase(leftit, afterit);
    tokens.insert(afterit, subexpr);

    return 1;
  }
  else
    return 0;
}

// parse parenthesis delimited expression
expr *parseto(const char *delim)
{
  expr *e = new expr;
  int not_eof = 0;
  do {
    not_eof = yylex();
    if(!strcmp(yytext, delim)) {
      while(1) {
	int flag = parseops(e->l);
	if(!flag) break;
      }
      expr *e2 = new expr;
      e2->l.push_back(new token(parentable[delim]));
      e2->l.push_back(e);
      return e2;
    }
    else if (!strcmp(yytext, ")"))
      e->l.push_back(parseto(")"));
    else if (!strcmp(yytext, "{"))
      e->l.push_back(parseto("}"));
    else
      e->l.push_back(new token(yytext));
  } while(not_eof);
  printf("Parse error: \"%s\" expected.\n", delim);
  return e;
}

// parse a line
void parseline(char *line)
{
  expr tokens;
  beginscanstring(line);
  int noteof = 0;
  do {
    noteof = yylex();
    if(noteof) {
      if (!strcmp(yytext, "("))
	tokens.l.push_back(parseto(")"));
      else if (!strcmp(yytext, "{"))
	tokens.l.push_back(parseto("}"));
      else
	tokens.l.push_back(new token(yytext));
    }
  } while(noteof);
  endscanstring();
  
  tokens.print();
  printf("\n");
  
  while(1) {
    int flag = parseops(tokens.l);
    if(!flag) break;
    tokens.print();
    printf("\n");
  }
}

