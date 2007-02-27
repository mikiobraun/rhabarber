/*
 * gc - garbage collector for C
 */

#include "gc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define gc_printdebug(...) /* nothing */

/* block header which is attached to every allocated block */
struct gc_memheader
{
  struct gc_memheader *prev, *next;
  unsigned marked : 1;
  int size;
  finalizer_t *finalizer;
  marker_t *marker;
};

#define MEMHEADER(p) ((p) ? (((struct gc_memheader*)(p)) - 1) : NULL)

GC_PS dynamic_head = { NULL, NULL };
GC_PS global_head = { NULL, NULL };

GC_PS *gc_cur_pframe = &dynamic_head;
GC_PS *global_pframe = &global_head;

int size_count;
int total_size_count;

int alloc_count;
int total_alloc_count;
struct gc_memheader alloc_list;

/* forward declarations */
static void insertheader(struct gc_memheader *mh1, struct gc_memheader *mh2);
static void delheader(struct gc_memheader *mh);
static void marklist(GC_PS *ps);

void gc_init()
{
  alloc_list.prev = &alloc_list;
  alloc_list.next = &alloc_list;
}


void *gc_malloc(size_t size)
{
  struct gc_memheader *mh = malloc(size + sizeof(struct gc_memheader));
  if(mh == NULL) { // collect and try again
    gc_collect();
    mh = malloc(size + sizeof(struct gc_memheader));
    if(mh == NULL) {
      fprintf(stderr, "GC: out of memory! Returning NULL");
      return NULL;
    }
  }
  insertheader(&alloc_list, mh);
  mh->marked = false;
  mh->size = size;
  mh->finalizer = NULL;
  mh->marker = NULL;

  // do some statistics
  alloc_count++;
  total_alloc_count++;
  size_count += size;
  total_size_count += size;

  gc_printdebug("allocated at %p\n", (void*)(mh + 1));

  return (void*)(mh + 1);
}


void gc_protect_global(void **name)
{
  GC_PS *ps = malloc(sizeof(GC_PS));
  ps->var = name;
  ps->next = NULL;
  global_pframe->next = ps;
  global_pframe = ps;
}


void gc_collect()
{
  struct gc_memheader *mh;

  gc_printdebug("Starting gc_collect()\n");

  // first mark everything as unmarked and unvisited
  for(mh = alloc_list.next; mh != &alloc_list; mh = mh->next) {
    gc_printdebug("    Unmarking %p\n", (void*)(mh + 1));
    mh->marked = false;
  }

  // now, start from the global variables and then mark the local variables
  gc_printdebug("  Marking globals\n");
  marklist(&global_head);
  gc_printdebug("  Marking dynamics\n");
  marklist(&dynamic_head);

  GC_PS *ps;
  for(ps = &dynamic_head; ps; ps = ps->next) {
    if(ps->var) {
      mh = MEMHEADER(*ps->var);
      if(mh) {
	mh->marked = true;
	// call mark function if one has been registered
	if(mh->marker)
	  mh->marker(mh + 1);
      }
    }
  }

  // everyone who has not been visited now gets deleted.
  int freed_count = 0;
  int freed_size = 0;
  alloc_count = 0;
  size_count = 0;
  struct gc_memheader *mh_next = mh->next;
  for(mh = alloc_list.next; mh != &alloc_list; mh = mh_next) {
    mh_next = mh->next;
    if(!mh->marked) {
      if(mh->finalizer)
	mh->finalizer(mh+1);
      freed_count = 0;
      freed_size = mh->size;
      delheader(mh);
      gc_printdebug("    Freeing %p\n", (void*)(mh + 1));
      free(mh);
    }
    else {
      alloc_count++;
      size_count += mh->size;
    }
  }
  fprintf(stderr, "GC_collect collected %d blocks, %.1fM\n",
	  freed_count, freed_size/1048576.0);
}


void gc_print_statistics()
{
  fprintf(stderr, "GC: blocks %d (total: %d), bytes %.1fM (total: %.1fM)\n", 
	  alloc_count, total_alloc_count, 
	  size_count/1048576.0, total_size_count/1048576.0);
}


void gc_register_finalizer(void *o, finalizer_t *f)
{
  MEMHEADER(o)->finalizer = f;
}

void gc_register_marker(void *o, marker_t *m)
{
  MEMHEADER(o)->marker = m;
}

void gc_mark(void *p)
{
  gc_printdebug("    Marking %p\n", p);
  MEMHEADER(p)->marked = true;
}

/*
 * private functions follow
 */

static void insertheader(struct gc_memheader *mh1, struct gc_memheader *mh2)
{
  mh2->next = mh1->next;
  mh2->prev = mh1;
  mh1->next->prev = mh2;
  mh1->next = mh2;
}

static void delheader(struct gc_memheader *mh)
{
  mh->next->prev = mh->prev;
  mh->prev->next = mh->next;
}


static void marklist(GC_PS *start)
{
  GC_PS *ps;
  struct gc_memheader *mh;
  for(ps = start; ps; ps = ps->next) {
    if(ps->var) {
      mh = MEMHEADER(*ps->var);
      if(mh) {
	gc_printdebug("    Marking %p\n", (void*)(mh + 1));
	mh->marked = true;
	// call mark function if one has been registered
	if(mh->marker)
	  mh->marker(mh + 1);
      }
    }
  }
}
