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


#include "utils.h"
//#define DEBUG
#include "debug.h"
#include "exception_tr.h"
#include "core.h"
#include "eval.h"

void rha_warning(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  print("warning: ");
  vprint(fmt, ap);
  va_end(ap);
}


void rha_error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char *msg = vsprint(fmt, ap);
  va_end(ap);

  if (eval_currentlocation) {
    object_t loc = rha_lookup(eval_currentlocation, rha_location_sym);
    if(loc) {
      msg = sprint("%o: %s", loc, msg);
      //      throw(_O exception_new(msg));
      throw(exception_new(msg));
    }
  }
  msg = sprint("error: %s", msg);
  //  throw(_O exception_new(msg));
  throw(exception_new(msg));
}

 
void sayhello()
{
  printf("Delicious rhabarber!\n"
	 "--RHA has been conceived by Stefan Harmeling and Mikio Braun.\n"
	 "--This interpreter was written by "
	 "Mikio Braun and Stefan Harmeling, 2005.\n"
	 "This is build $Revision: 324 $\n"
	 "              "
	 "$Date: 2006-08-02 02:04:16 +0200 (Wed, 02 Aug 2006) $\n"
	 "Compiled      " __DATE__ ", " __TIME__ "\n\n");
}


void saybye()
{
  printf("\nBye.\n");
}

