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

extern _rha_ void warning(string_t);
extern _rha_ void error(string_t);
extern void rha_warning(const char *fmt, ...);
extern void rha_error(const char *fmt, ...);
extern void rha_errorloc(any_t expr, const char *fmt, ...);

extern void sayhello();
extern void saybye();

#endif
