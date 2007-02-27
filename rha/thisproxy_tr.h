/*
 * thisproxy - [enter description here]
 *
 * This file is part of rhabarber.
 *
 * (c) 2005 by Mikio Braun,        Stefan Harmeling,
 *             mikio.fhg.de  harmeli.fhg.de
 *                             
 */

#ifndef THISPROXY_H
#define THISPROXY_H

#include "primtype.h"
#include "object.h"

typedef struct thisproxy_s *thisproxy_tr;

extern primtype_t thisproxy_type;

extern void thisproxy_init(void);
extern constructor thisproxy_tr thisproxy_new(object_t this);

extern method string_t thisproxy_to_string(thisproxy_tr);

#endif
