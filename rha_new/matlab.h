/*
 * matlab - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#ifndef MATLAB_H
#define MATLAB_H

#include <matrix.h>

#include "primtype.h"
#include "object.h"
#include "tuple_tr.h"

typedef struct matlab_s *matlab_tr;
typedef unsigned long varid_t;

extern primtype_t matlab_type;
extern matlab_tr matlab_domain;

extern void matlab_init(void);

extern varid_t matlab_newvarid();
extern void matlab_clear(varid_t vi);
extern void matlab_put(varid_t vi, mxArray *a);
extern mxArray *matlab_get(varid_t vi);

extern string_t matlab_varidname(varid_t vi);

/*
 * matlab is a singleton, which means that all methods here are
 * "constructors" because they do not take an object argument
 */ 

extern constructor matlab_tr matlab_new();
extern constructor void matlab_evalstring(string_t);

extern constructor object_t matlab_newinteger(int i);
extern constructor object_t matlab_newreal(double r);
extern constructor object_t matlab_newbool(bool b);
extern constructor object_t matlab_newstring(string_t b);
extern constructor object_t matlab_newmatrix(int rows, int cols);

extern constructor object_t matlab_call(string_t fn, int outargs,
					object_t args);

#endif
