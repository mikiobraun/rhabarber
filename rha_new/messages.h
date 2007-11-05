/*
 * messages - support for warnings and error messages
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include "object.h"

extern void warning(string_t);
extern void error(string_t);
extern _ignore_ void rha_warning(const char *fmt, ...);
extern _ignore_ void rha_error(const char *fmt, ...);
extern _ignore_ void rha_errorloc(any_t expr, const char *fmt, ...);

extern void sayhello(void);
extern void saybye(void);

#endif
