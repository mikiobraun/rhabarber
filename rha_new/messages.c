/*
 * messages - support for warnings and error messages
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#include "messages.h"

#include "rha_types.h"
#include "utils.h"
//#define DEBUG
#include "debug.h"
#include "core.h"
#include "eval.h"
#include "excp.h"

void rha_warning(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "[warning] %s", vsprint(fmt, &ap));
  va_end(ap);
}


void rha_error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  string_t msg = vsprint(fmt, &ap);
  va_end(ap);

  msg = sprint("[error] %s", msg);
  throw(excp_new(msg));
}

 
void sayhello()
{
  printf("\n"
	 "Delicious rhabarber!\n"
	 "--conceived by Stefan Harmeling and Mikio Braun\n"
	 "--written by Mikio Braun and Stefan Harmeling, 2005, 2006, 2007\n"
	 "--compiled at " __TIME__ " on " __DATE__ "\n");
}


void saybye()
{
  printf("\nBye.\n");
}

