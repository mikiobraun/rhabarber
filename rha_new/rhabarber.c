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

  // load basic stuff implemented in rhabarber
  string_t fname = "prelude.rha";
  object_t excp;   // exception object
  try { 
    object_t e = run_fn(root, fname);
    if (!e) 
      rha_error("can't load '%s'", fname); 
    print("--loaded and run \"%s\"\n", fname);
  } 
  catch(excp) { 
    excp_show(excp);
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
	object_t e = eval(root, p);
	fprint(stdout, "%o\n", e);
      }
      else
	rha_error("parse returned ZERO");
    }
    catch(excp) {
      excp_show(excp);
    }
  }

  // so long
  saybye();
}
