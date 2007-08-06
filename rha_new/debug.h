/*
 * debug - debugging support
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio@first.fhg.de  harmeling@gmail.com
 *                             
 */

#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG
// if you want to have debugging information, uncomment this
//#define DEBUG
#endif

/* printdebug is like printf but will be suppressed if DEBUG is not
   defined. */
#ifdef DEBUG
extern void _printdebug(const char *fmt, ...);
#define printdebug(fmt, ...) _printdebug("%s:%d: " fmt, __func__, __LINE__, __VA_ARGS__);
#else
#define printdebug(...) /* nothing */
#endif

#endif

