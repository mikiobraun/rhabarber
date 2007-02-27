/*
 * mxfunc - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#ifndef MXFUNC_H
#define MXFUNC_H

#include "primtype.h"
#include "object.h"

typedef struct mxfunc_s *mxfunc_tr;

extern primtype_t mxfunc_type;

extern void mxfunc_init(void);

extern constructor mxfunc_tr mxfunc_new(string_t name);

#endif
