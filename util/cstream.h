/*
 * cstream - streams for C facility
 *
 * Provides a uniform interface for outputting character and strings
 * to files, strings, etc.
 *
 * (c) 2005 by Mikio L. Braun, mikio@first.fhg.de
 */

#ifndef CSTREAM_H
#define CSTREAM_H

#include <stdio.h>

/*
 * stream contains a table of function pointers which specify how the
 * basic operations should be performed
 */
struct stream
{
  void (*strputc)(void *s, char c); // fn to put a character
  void (*strclose)(void *s);        // fn to close
};
typedef struct stream STREAM;

/* open a stream to a file */
extern STREAM *fstropen(FILE *f);

/* open a stream to a string.
   if str == NULL, a buffer is allocated which is freed on strclose()  
   if len == 0 and str == NULL, the buffer grows automatically. */
extern STREAM *sstropen(char *str, int len);

/* put a character to the stream */
extern void strputc(STREAM *s, int ch);

/* put a null-terminated string to the stream */
extern void strputs(STREAM *s, const char *str);

/* close the stream. This might free a local buffer, but does not
   close the file for file streams */
extern void strclose(STREAM *s);

/* get the buffer for string streams */
extern char *sstrbuffer(STREAM *s);

/* this is the standard out stream */
extern STREAM *cout;
extern STREAM *cerr;

#endif
