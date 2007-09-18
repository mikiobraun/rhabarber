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
#include <getopt.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <gc/gc.h>

#include "messages.h"
#include "rha_parser_fcts.h" // for 'rhaparsestring'
#include "utils.h"
#include "excp.h"
#include "rha_init.h"
#include "utilalloc.h"

int main(int argc, char **argv)
{
  // handle command line arguments
  int flag;
  bool_t dflag = false, tflag = false;
  bool_t interactive = false;
  while ((flag = getopt(argc, argv, "dhti")) != -1)
    switch (flag) {
    case 'h':  // show command line help
      printf("\n"
	     "Rhabarber, compiled at " __TIME__ " on " __DATE__ "\n"
	     "\n"
	     "usage: rhabarber [-dht] [file]\n"
	     "\n"
	     "  d : do not load 'prelude.rha'\n"
	     "  h : print this help message and ignore other options\n"
	     "  t : load 'prelude.rha', run 'test.rha' and quit\n"
	     "  i : enter interactive loop after [file] has been read\n"
	     "\n");
      exit(0);
    case 'd':  // debug mode, i.e. without anything else loaded
      dflag = true;
      break;
    case 't':  // test mode, run 'prelude.rha' and 'test.rha' and quit
      tflag = true;
      break;
    case 'i': 
      interactive = true;
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
  any_t root = rha_init();
  any_t excp = 0;   // exception object

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

  if (optind < argc) {
    try {
      for(int i = optind; i < argc; i++)
	run_fn(root, argv[optind]);
    }
    catch(excp) {
      excp_show(excp);
    }
    if(!interactive)
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
      // parse split by semicolon (and other tricks)
      any_t value = split_resolve_and_eval(root, rhaparsestring(line), 0);

      // and print the result
      if (value != void_obj)
	fprintf(stdout, "%s\n", to_string(value));
    }
    catch(excp) {
      excp_show(excp);
    }
  }

  // so long
  saybye();
}
