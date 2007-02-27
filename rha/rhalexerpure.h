#ifndef RHALEXERPURE_H
#define RHALEXERPURE_H

#include <stdio.h>
#include "object.h"

extern object_t yypurelval;
extern char *yypuretext;
extern int yypureleng;

extern int get_next_token();
extern void begin_scanning_string(const char *s);
extern void begin_scanning_file(FILE *);
extern void end_scanning();

extern int fileno(FILE *);

#endif
