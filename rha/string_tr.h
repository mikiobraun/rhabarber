#ifndef STRING_TR_H
#define STRING_TR_H

#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"

typedef struct string_s *string_tr;
extern primtype_t string_type;
extern void string_init(void);

extern constructor string_tr string_new(string_t text);
extern method string_tr string_copy(string_tr s);

extern method string_t string_get(string_tr s);
extern method int string_length(string_tr s);

extern operator string_t string_plus_string(string_t s1, string_t s2);
extern operator bool string_less_string(string_t s1, string_t s2);
extern operator bool string_greater_string(string_t s1, string_t s2);
extern operator bool string_equal_string(string_t s1, string_t s2);

extern method string_t string_to_string(string_tr s);

#endif
