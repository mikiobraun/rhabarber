/*
 * mxarray - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#ifndef MXARRAY_H
#define MXARRAY_H

#include <matrix.h>

#include "primtype.h"
#include "object.h"
#include "matlab_tr.h"

typedef struct mxarray_s *mxarray_tr;

extern primtype_t mxarray_type;

extern void mxarray_init(void);

extern mxarray_tr mxarray_new(mxArray *a, varid_t varid);

// accessors to the object
extern mxArray *mxarray_array(mxarray_tr);
extern varid_t mxarray_varid(mxarray_tr);

// constructing a new variable in matlab
extern constructor mxarray_tr mxarray_newvar();

// object information
extern method string_t mxarray_varidname(mxarray_tr);
extern method string_t mxarray_typeid(mxarray_tr);

// moving objects from/to matlab
extern method mxarray_tr mxarray_putobj(mxarray_tr a);
extern method mxarray_tr mxarray_getobj(mxarray_tr a);

// accessing matrix elements / size information
extern method int mxarray_rows(mxarray_tr a);
extern method int mxarray_cols(mxarray_tr a);
extern method object_t mxarray_get(mxarray_tr, int i, int j);
extern method object_t mxarray_set(mxarray_tr, int i, int j, object_t o);

// getting scalar values
extern method int mxarray_getint(mxarray_tr a);
extern method double mxarray_getreal(mxarray_tr a);

// calling binary and unary operators
extern constructor object_t mxarray_binop(string_t op, mxarray_tr arg1, 
					  mxarray_tr arg2);
extern constructor object_t mxarray_unop(string_t op, mxarray_tr arg);

// other rhabarber-related methods
extern method string_t mxarray_to_string(mxarray_tr a);

#endif
