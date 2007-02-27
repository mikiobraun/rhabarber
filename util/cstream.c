/*
 * cstream
 *
 * some facility for doing target-independent character outputting.
 */

/*
 * This file is structured as follows:
 *  
 *   Definition of data types
 *   Forward declarations of local functions
 *   General functions (the interface)
 *   Implementation specific functions for the different types
 */

#include "cstream.h"

#include <stdlib.h>
#include <stdbool.h>

#ifndef ALLOC
#define ALLOC(p) p = malloc(sizeof(*p))
#endif


#define BUFSIZE 256
/*
 * data types
 */

struct fstream
{
  struct stream table;
  enum { FS_STDOUT, FS_STDERR, FS_FILE } selector;
  FILE *f;
};

struct sstream
{
  struct stream table;
  char *buffer;
  int len;
  int pos;
  bool grow_buffer;
  bool free_buffer;
};


/*
 * Forward declarations
 */

void fsputc(void *s, char c);
void fsclose(void *s);
void ssputc(void *s, char c);
void ssclose(void *s);

static
struct fstream cout_value = {
  { fsputc, NULL }, FS_STDOUT, NULL
};

STREAM *cout = (STREAM*)&cout_value;


static
struct fstream cerr_value = {
  { fsputc, NULL }, FS_STDERR, NULL
};

STREAM *cerr = (STREAM*)&cerr_value;

/*
 * general functions
 */

/* open a stream with an existing file handle */
STREAM *fstropen(FILE *f)
{
  struct fstream *s;
  ALLOC(s);
  s->table.strputc = fsputc;
  s->table.strclose = fsclose; 
  s->selector = FS_FILE;
  s->f = f;
  return (STREAM*)s;
}


/* open a stream with a string buffer */
STREAM *sstropen(char *str, int len)
{
  struct sstream *s;
  ALLOC(s);
  s->table.strputc = ssputc;
  s->table.strclose = ssclose; 

  /* there are three different cases
   *
   * str != NULL: use external buffer, don't grow
   * str == NULL, len > 0: use internal buffer, don't grow
   * str == NULL, len == 0: use internal buffer which grows
   */
  if (str != NULL) {
    s->buffer = str;
    s->len = len;
    s->free_buffer = false;
    s->grow_buffer = false;
  }
  else {
    if (len > 0) {
      s->buffer = malloc(len);
      s->len = len;
      s->free_buffer = true;
      s->grow_buffer = false;
    }
    else {
      s->buffer = malloc(BUFSIZE);
      s->len = BUFSIZE;
      s->free_buffer = true; 
      s->grow_buffer = true;
    }
  }
  s->buffer[0] = '\0'; // just in case nothing gets added
  s->pos = 0;
  return (STREAM*)s;
}


/* print a character to stream */
void strputc(STREAM *s, int ch)
{
  s->strputc(s, ch);
}


/* print a string to stream */
void strputs(STREAM *s, const char *str)
{
  while(*str)
    strputc(s, *str++);
}


/* close the stream */
void strclose(STREAM *s)
{
  if(s->strclose) s->strclose(s);
}

/*
 * implementation specific functions
 */ 

/* put character to file handle */
void fsputc(void *p, char c)
{
  struct fstream *s = p;
  if (s->f != NULL)
    fputc(c, s->f);
  else
    putchar(c);
}

/* close stream to file. Frees the structure but does not close
   file */
void fsclose(void *p)
{
  free(p);
}

/* put character to string buffer. Checks for overflow */
void ssputc(void *p, char c)
{
  struct sstream *s = p;
  if (s->pos == s->len - 1 &&
      s->grow_buffer) {
    s->buffer = realloc(s->buffer, s->len + BUFSIZE);
    s->len += BUFSIZE;
  }

  if (s->pos < s->len - 1) {
    s->buffer[s->pos++] = c;
    s->buffer[s->pos] = '\0';
  }
}


void ssclose(void *p)
{
  struct sstream *s = p;
  if (s->free_buffer)
    free(s->buffer);
  free(p);
}


char *sstrbuffer(STREAM *p)
{
  struct sstream *s = (struct sstream*)p;
  return s->buffer;
}
