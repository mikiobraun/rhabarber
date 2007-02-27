#ifndef NONE_TR_H
#define NONE_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"

typedef struct none_s *none_tr;

extern primtype_t none_type;
extern none_tr none_obj;

extern void none_init(void);

extern constructor none_tr none_new();
extern method string_t     none_to_string(none_tr b);
extern method bool         none_is_none(none_tr b);
extern operator bool       none_equal_none(none_tr n1, none_tr n2);

#endif
