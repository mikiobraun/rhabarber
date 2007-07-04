/*
 * rhabarber - main() and read-eval loop
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "rhaparser_extra.h"
#include "messages.h"
#include "object.h"
#include "prule.h"
#include "eval.h"
#include "symtable.h"
#include "utils.h"

//#define DEBUG
#include "debug.h"

// to be initalized
#include "core.h"
#include "builtin_tr.h"
#include "function_tr.h"
#include "overloaded_tr.h"
//#include "method_tr.h"
//#include "graphics.h"
#include "exception_tr.h"

#include "tuple_tr.h"
#include "list_tr.h"
#include "dict_tr.h"
#include "listit_tr.h"

#include "plain_tr.h"
#include "none_tr.h"
#include "symbol_tr.h"
#include "string_tr.h"
#include "bool_tr.h"
#include "int_tr.h"
#include "real_tr.h"
#include "complex_tr.h"
#include "matrix_tr.h"
#include "parser_tr.h"

// next come the stubs
#include "tuple_stub.h"
#include "list_stub.h"
#include "listit_stub.h"
#include "dict_stub.h"

#include "plain_stub.h"
#include "none_stub.h"
#include "symbol_stub.h"
#include "string_stub.h"
#include "bool_stub.h"
#include "int_stub.h"
#include "real_stub.h"
#include "complex_stub.h"
#include "matrix_stub.h"
#include "builtin_stub.h"
#include "function_stub.h"
#include "overloaded_stub.h"
//#include "method_stub.h"
#include "exception_stub.h"
#include "parser_stub.h"

#ifdef HAVE_PYTHON
#  include "python_tr.h"
#  include "python_stub.h"
#  include "pyobject_tr.h"
#  include "pyobject_stub.h"
#endif
//#include "thisproxy_tr.h"
//#include "thisproxy_stub.h"
#ifdef HAVE_MATLAB
#  include "matlab_tr.h"
#  include "matlab_stub.h"
#  include "mxarray_tr.h"
#  include "mxarray_stub.h"
#  include "mxfunc_tr.h"
#  include "mxfunc_stub.h"
#endif

// forward declarations
void read_eval_loop();
void init(int argc, char *argv[]);
object_t init_stubs(void);

/* the main function
 *
 * initializes the system, parses the command line, executes any files
 * passed on the command line, and then enters the main read-eval
 * loop.
 */
int main(int argc, char *argv[])
{
  // initialize garbage collection
  GC_INIT();
  GC_set_max_heap_size(512*1048576);
  GC_enable();

  // initialize objects
  init(argc, argv);

  // initialize rhabarber
  object_t root = init_stubs();

# ifdef HAVE_PYTHON
  python_init();
  pyobject_init();
  python_stub_init(root);
  pyobject_stub_init(root);
# endif

# ifdef HAVE_MATLAB
  matlab_init();
  matlab_stub_init(root);
  mxarray_init();
  mxarray_stub_init(root);
  mxfunc_init();
  mxfunc_stub_init(root);
# endif

  // execute string
  if (argc > 1) {
    if (strcmp(argv[1], "-x") == 0 && argc > 2) {
      // execute the next arg
      list_tr code = rhaparsestring(argv[2]);
      object_t excp;
      try {
	object_t o = resolve_eval_list(root, code);
	if(o) fprint(stdout, "%o\n", o);
      }
      catch(excp) {
	printdebug("exception == %x\n", excp);
	fprint(stderr, "Caught exception: %o\n", excp);
      }
      exit(EXIT_SUCCESS);
    }
  }

  // use history for the shell
  using_history();

  // be polite
  sayhello();

  // load files listed in the command line
  if(argc > 1) {
    for(int i = 1; i < argc; i++) {
      fprint(stdout, "--loading \"%s\"\n", argv[i]);
      list_tr code = rhaparsefile(argv[i]);
      if(code) {
	object_t excp;
	try 
	  resolve_eval_list(root, code);
	catch(excp) {
	  printdebug("exception == %x\n", excp);
	  fprint(stderr, "Caught exception: %o\n", excp);
	}
      }
      else
	fprint(stderr, "--error loading \"%s\"\n", argv[i]);
    }
  }

  // the read eval loop
  eval_currentlocation = NULL;
  read_eval_loop(root);

  // so long
  saybye();
}


/* This global variable points to the environment in which the current
 * evaluation takes place
 */
object_t complenv;


/* Hook function for readline.
 *
 * This function is called by readline to provide possible completion
 * for a partially entered string. This function can also parse a
 * nested expression like "x.a.b". It will then search for extensions
 * of "b" in the object x.a
 */
char* rhabarber_completion_generator(const char *txt, int count)
{
  static tuple_tr matches;
  static int mycount;
  static string_t path, stxt;
  static int length; 

  // initialize
  //
  // If count == 0, this is the first time, and we are parsing the
  // path and collect the slots contained in a function.
  if (!count) {
    // In case the string contains dots "." try to lookup the
    // path.
    char *prefix = gc_strdup(txt);
    char *start = prefix;
    char *end;
    object_t obj = complenv;

    while(obj && start && (end = strchr(start, '.'))) {
      *end = '\0';       // cut out the first name
      obj = rha_lookup(obj, symbol_new(start));
      *end = '.';        // reconstruct the dot
      start = end + 1;   // and start at the next word
    }

    if (!obj) // there was a prefix present, but we couldn't interpret it
      return NULL;

    if (start != prefix) {
      start[-1] = '\0'; // cut out the whole prefix
      path = string_plus_string(prefix, ".");
    }
    else
      path = "";
    
    stxt = start;
    
    // collect matches
    matches = list_to_tuple(object_allslots(obj));

    // set up variables
    mycount = 0;
    length = strlen(stxt);
  }

  // search next matching substring
  for(;mycount < tuple_length(matches); mycount++) {
    string_t c = symbol_name(tuple_get(matches, mycount));

    if (!length || strncmp(stxt, c, strlen(stxt)) == 0) {
      mycount++;
      return nongc_strdup(string_plus_string(path, c));
    }
  }
  return NULL;
}


/* the main read-eval loop
 *
 * reads a string using the readline library, evaluates the string in
 * env and then prints the result.
 */
void read_eval_loop(object_t env) {
  
  char prompt[100];
  int lineno = 1;

  // setup readline
  rl_completion_entry_function = rhabarber_completion_generator;
  rl_basic_word_break_characters = " +*\t\n<>{}()";
  rl_completion_append_character = '\0';

  while(1) {
    // read line
    sprintf(prompt, "rha[%d]$ ", lineno);
    complenv = env;
    char *line = readline(prompt);
    if (!line) break;
    add_history(line);
    
    // opengl
    //graphics_display();
    
    object_t excp;
    try {
      // process line
      list_tr expr_list = rhaparsestring(line);
      if(expr_list) {
	//object_print(expr_list);
	eval_currentlocation = NULL;
	object_t o = resolve_eval_list(env, expr_list);
	//if(o && !HAS_TYPE(none, o))
	//  print("%o\n", o);
	if(o) print("%o\n", o);
      }
    }
    catch(excp) {
      printdebug("exception == %x\n", excp);
      fprint(stderr, "Caught exception: %o\n", excp);
    }

    // opengl
    //graphics_display();
    
    // cleanup & next round
    free(line);
    lineno++;
  }
}


/* initialize the modules
 */
void init(int argc, char *argv[])
{
  // 'plain' must be first
  plain_init();

  // the order of the rest does not matter

  // deeper stuff
  core_init();
  builtin_init();
  function_init();
  overloaded_init();
  symbol_init();
  //method_init();
  //graphics_init(argc, argv);
  exception_init();
  //thisproxy_init();

  // containers
  tuple_init();
  list_init();
  listit_init();
  dict_init();

  // ordinary types
  none_init();
  string_init();
  bool_init();
  int_init();
  real_init();
  complex_init();
  matrix_init();
  parser_init();
}


/* initialize the stubs
 *
 * In constrast to the *_init() functions, the stubs mainly construct
 * the underlying the rhabarber-side objects, and slots for the
 * modules' data types.
 */
object_t init_stubs(void)
{
  // core goes first and creates 'root'
  object_t root =   core_stub_init();

  // the order of the stubs does not matter 
  // therefore, it matches the order of the init's()

  overloaded_stub_init(root);

  plain_stub_init(root);

  // deeper
  builtin_stub_init(root);
  function_stub_init(root);
  //method_stub_init(root);
  //graphics_stub_init(root);
  exception_stub_init(root);
  //thisproxy_stub_init(root);

  // containers
  tuple_stub_init(root);
  list_stub_init(root);
  listit_stub_init(root);
  dict_stub_init(root);

  // ordinary
  none_stub_init(root);
  symbol_stub_init(root);
  string_stub_init(root);
  bool_stub_init(root);
  int_stub_init(root);
  real_stub_init(root);
  complex_stub_init(root);
  matrix_stub_init(root);
  parser_stub_init(root);

  return root;
}


