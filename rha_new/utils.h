/*
 * utils - utility functions
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include "rha_types.h"

extern int_t idx(int_t i, int_t len);

extern void print(const char *fmt, ...);
extern void vprint(const char *fmt, va_list *ap);
extern void fprint(FILE *f, const char *fmt, ...);
extern void vfprint(FILE *f, const char *fmt, va_list *ap);
extern char *sprint(const char *fmt, ...);
extern char *vsprint(const char *fmt, va_list *ap);

extern void printusage(char *str);

#endif
