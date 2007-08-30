/*
 * rhabarber - the main file
 *
 * This file is part of rhabarber.
 *
 * (c) 2005-2007 by Mikio Braun          & Stefan Harmeling
 *                  mikiobraun@gmail.com   harmeling@gmail.com
 *                             
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <gc/gc.h>

#include "messages.h"
#include "utils.h"
#include "excp.h"
#include "rha_init.h"
#include "utilalloc.h"

int main(int argc, char **argv)
{
  // handle command line arguments
  int flag;
  bool_t dflag = false, tflag = false;
  while ((flag = getopt(argc, argv, "dht")) != -1)
    switch (flag) {
    case 'h':  // show command line help
      printf("\n"
	     "Rhabarber, compiled at " __TIME__ " on " __DATE__ "\n"
	     "\n"
	     "usage: rhabarber [-dht]\n"
	     "\n"
	     "  d : do not load 'prelude.rha'\n"
	     "  h : print this help message and ignore other options\n"
	     "  t : load 'prelude.rha', run 'test.rha' and quit\n"
	     "\n");
      exit(0);
    case 'd':  // debug mode, i.e. without anything else loaded
      dflag = true;
      break;
    case 't':  // test mode, run 'prelude.rha' and 'test.rha' and quit
      tflag = true;
      break;
    }

  // initialize garbage collection
  GC_INIT();
  GC_set_max_heap_size(512*1048576);
  util_malloc = GC_malloc;
  util_free = GC_free;

  // use history for the shell
  using_history();

  // be polite
  sayhello();

  // set up root object
  object_t root = rha_init();
  object_t excp = 0;   // exception object

  if (!dflag) {
    // load basic stuff implemented in rhabarber
    string_t fname = "prelude.rha";
    try { 
      run_fn(root, fname);
      print("--loaded and run \"%s\"\n", fname);
    }
    catch(excp) { 
      excp_show(excp);
    } 
  }
  if (tflag) {
    string_t fname = "test.rha";
    try { 
      run_fn(root, fname);
      print("--loaded and run \"%s\"\n", fname);
    }
    catch(excp) { 
      excp_show(excp);
    }
    exit(0);
  }    

  // for the prompt
  int lineno = 0;
  char prompt[1024];

  // the read eval print loop (REPL)
  // never returns
  print("\n");
  while(1) {
    // read line
    sprintf(prompt, "rha[%d](%d)$ ", lineno++, frame_tos);
    string_t line = readline(prompt);
    if (!line) break;
    add_history(line);
    try {
      object_t p = parse(root, line);
      if (p) {
	// note that the outer tuple containing a "do" is dealt with
	// here, because this way we can avoid opening a BLOCK_FRAME.
	// this makes sure that at the prompt 'deliver 17' will issue
	// an error as wanted
	assert(ptype(p) == TUPLE_T);
	tuple_t t = UNWRAP_PTR(TUPLE_T, p);
	int_t tlen = tuple_len(t);
	assert(tlen > 0);
	assert(ptype(tuple_get(t, 0)) == SYMBOL_T);
	assert(UNWRAP_SYMBOL(tuple_get(t, 0)) == do_sym);
	object_t e = 0;
	for (int i = 1; i < tlen; i++)
	  e = eval(root, tuple_get(t, i));
	if (e)
	  fprint(stdout, "%o\n", e);
      }
    }
    catch(excp) {
      excp_show(excp);
    }
  }

  // so long
  saybye();
}
