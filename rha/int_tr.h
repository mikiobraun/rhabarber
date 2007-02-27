#ifndef INT_TR_H
#define INT_TR_H

#include <stdbool.h>
#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"

typedef struct int_s *int_tr;
extern primtype_t int_type;
extern void int_init(void);

extern constructor int_tr int_new(int value);
extern method int_tr      int_copy(int_tr i);

extern method int         int_get(int_tr i);
extern method double      int_exp(int i);
extern method double      int_sin(int i);
extern method double      int_sinc(int i);
extern method int         int_inc(int i);
extern method int         int_dec(int i);

/* prefix operators */
extern operator int       int_minus(int i);

/* infix operators */
extern operator int       int_plus_int(int i1, int i2);
extern operator int       int_minus_int(int i1, int i2);
extern operator int       int_times_int(int i1, int i2);
extern operator int       int_divide_int(int i1, int i2);
extern operator bool      int_less_int(int i1, int i2);
extern operator bool      int_greater_int(int i1, int i2);
extern operator bool      int_equal_int(int i1, int i2);

extern method string_t    int_to_string(int i);

#endif
