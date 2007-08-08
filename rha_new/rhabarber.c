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

#include "rha_init.h"

int main(int argc, char **argv)
{
  // initialize garbage collection
  GC_INIT();
  GC_set_max_heap_size(512*1048576);
  GC_enable();

  // use history for the shell
  using_history();

  // set up root object
  object_t root = rha_init(root);
  
  int lineno = 0;

  // the read eval print loop (REPL)
  // never returns
  while(1) {
    // read line
    sprintf(prompt, "rha[%d]$ ", lineno);
    char *line = readline(prompt);
    if (!line) break;
    add_history(line);

    object_t p = parse(line);
    object_t e = eval(p);
    print(p);
  }
}  
