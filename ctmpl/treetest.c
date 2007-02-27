#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "treestringint.h"

extern char *strdup(char *s);

int main(int argc, char **argv)
{
  // inputs
  int numwords = 10;    // number of words
  if (argc>1) numwords = atoi(argv[1]);

  int numqueries = 10;   // number of queries
  if (argc>2) numqueries = atoi(argv[2]);

  int numops = 10;   // number of random operations
  if (argc>3) numops = atoi(argv[3]);

  srand(time(0));   // seed for rand()
  if (argc>4) srand(atoi(argv[4]));

  // some large storage
  int maxmem = 1000000;
  char *mem = malloc(maxmem*sizeof(char));
  char **memptrs = malloc(numwords*sizeof(char *));
  memptrs[0] = mem;

  // load words
  FILE *fp = fopen("/usr/share/dict/words","r");
  int maxline = 256;
  char line[maxline];
  int i;
  for (i = 0; i < numwords; i++) {
    if (!fgets(line, maxline, fp)) break;
    // get rid of the "\n"
    line[strlen(line)-1] = 0;

    // store the new word in some unorganized memory
    // which we can access with random numbers
    strcpy(memptrs[i], line);
    memptrs[i+1] = memptrs[i] + strlen(line) + 1;  // the next location
    if (memptrs[i+1] >= mem + maxmem) break;
  };
  fclose(fp);
  numwords = i;  // update numwords
  
  // FIRST INSERTING IN ORDER THEN RANDOM QUERYING
  // initialize the tree and fill it with words
  treestringint_t t;
  treestringint_init(&t);
  bool exists[numwords];
  int r;
  for (i = 0; i < numwords; i++) {
    // insert a word in the dictionary
    treestringint_insert(&t, strdup(memptrs[i]), i);
    exists[i] = true;
  };

  // test the dictionary
  int depths[(numqueries>numops) ? numqueries : numops];
  int val;
  for (i = 0; i < numqueries; i++) {
    r = rand() % numwords;
    if (r != treestringint_search(&t, memptrs[r])) {
      if (exists[r]) {
	printf("SEARCH_ERROR\n");
	abort();
      }
    }
    depths[i] = treestringint_lastsearchdepth;
  }

  // statistics
  int sum = 0;
  int mind = numwords;
  int maxd = 0;
  int d;
  for (i = 1; i < numqueries; i++) {
    d = depths[i];
    sum += d;
    if (d < mind) mind = d;
    if (d > maxd) maxd = d;
  }
  printf("\nFIRST INSERTING THEN QUERYING:\n");
  printf("number of words = %d\n", numwords);
  printf("number of queries = %d\n", numqueries);
  printf("minimum depth = %d\n", mind);
  printf("average depth = %f\n", 1.0*sum/numqueries);
  printf("maximum depth = %d\n", maxd);


  // RANDOM OPERATIONS
  treestringint_clear(&t);
  int numinserts = 0;
  numqueries = 0;
  int numdeletes = 0;
  for (i = 0; i < numwords; i++) exists[i] = false;
  i = 0;
  while (i < numops) {
    // decide randomly what to do
    float p = 1.0*rand()/RAND_MAX;
    if (p > 0.8) {
      // delete an entry
      r = rand() % numwords;
      while (!exists[r] && r < numwords) r++;
      if (r < numwords) {
	val = treestringint_delete(&t, memptrs[r]);
	if (val != 0) {
	  if (val != r) {
	    printf("DELETE_ERROR\n");
	    abort();
	  }
	  else {
	    numdeletes++;
	    exists[r] = false;
	    i++;
	  }
	}
      }
    }
    else if (p > 0.5) {
      r = rand() % numwords;
      // insert a word in the dictionary
      treestringint_insert(&t, strdup(memptrs[r]),r);
      exists[r] = true;
      numinserts++;
      i++;
    } 
    else {
      // query a word
      r = rand() % numwords;
      if (r != treestringint_search(&t, memptrs[r])) {
	if (exists[r]) {
	  printf("SEARCH_ERROR\n");
	  abort();
	}
      }
      depths[numqueries] = treestringint_lastsearchdepth;
      numqueries++;
      i++;
    }
  }
  
  // statistics
  sum = 0;
  mind = numinserts;
  maxd = 0;
  for (i = 1; i < numqueries; i++) {
    d = depths[i];
    sum += d;
    if (d < mind) mind = d;
    if (d > maxd) maxd = d;
  }
  printf("\nRANDOM OPERATIONS:\n");
  printf("number of operations = %d\n", numops);
  printf("number of inserts = %d\n", numinserts);
  printf("number of queries = %d\n", numqueries);
  printf("number of deletes = %d\n", numdeletes);
  printf("minimum depth = %d\n", mind);
  if (numqueries > 0) printf("average depth = %f\n", 1.0*sum/numqueries);
  else printf("average depth = NO QUERIES\n");
  printf("maximum depth = %d\n", maxd);

  return 0;
}
