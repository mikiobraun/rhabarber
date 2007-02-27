#ifndef BOOL_TR_H
#define BOOL_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"
#include "none_tr.h"

typedef struct bool_s *bool_tr;

extern primtype_t bool_type;

extern void bool_init(void);

extern constructor bool_tr bool_new(bool val);
extern method bool_tr      bool_copy(bool_tr b);
extern method bool         bool_get(bool_tr b);

extern operator bool       bool_equal_bool(bool b1, bool b2);
extern operator bool       bool_not(bool b);
// 'and' and 'or' are now with lazy macro-versions in core.c
extern method   string_t   bool_to_string(bool b);

extern constructor bool    bool_from_none(none_tr n);

#endif
