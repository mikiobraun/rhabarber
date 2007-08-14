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

void rha_warning(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "[warning] %s", vsprint(fmt, ap));
  va_end(ap);
}


void rha_error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  string_t msg = vsprint(fmt, ap);
  va_end(ap);

  msg = sprint("[error] %s", msg);
  throw(WRAP_PTR(STRING_T, string_proto, msg));
}

 
void sayhello()
{
  printf("Delicious rhabarber!\n"
	 "--The language has been conceived by Stefan Harmeling and Mikio Braun.\n"
	 "--This interpreter was written by "
	 "Mikio Braun and Stefan Harmeling, 2005, 2006, 2007.\n"
	 //"This is build $Revision: 324 $\n"
	 //"              "
	 //"$Date: 2006-08-02 02:04:16 +0200 (Wed, 02 Aug 2006) $\n"
	 "Compiled      " __DATE__ ", " __TIME__ "\n\n");
}


void saybye()
{
  printf("\nBye.\n");
}

