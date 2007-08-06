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

extern void rha_warning(const char *fmt, ...);
extern void rha_error(const char *fmt, ...);
extern void rha_errorloc(object_t expr, const char *fmt, ...);

extern void sayhello();
extern void saybye();

#endif
