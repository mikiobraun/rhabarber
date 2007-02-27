#ifndef REAL_TR_H
#define REAL_TR_H

#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"
#include "int_tr.h"
#include "complex_tr.h"

typedef struct real_s *real_tr;
extern primtype_t real_type;
extern void real_init();

extern constructor real_tr real_new(double value);
extern constructor double  real_from_int(int i);
extern method real_tr      real_copy(real_tr r);

extern method double       real_get(real_tr r);
extern method object_t     real_sqrt(double d);
extern method double       real_exp(double d);
extern method double       real_sin(double d);
extern method double       real_sinc(double d);

extern operator double     real_minus(double d);
extern operator double     real_plus_real(double d1, double d2);
extern operator double     real_minus_real(double d1, double d2);
extern operator double     real_times_real(double d1, double d2);
extern operator double     real_divide_real(double d1, double d2);

extern operator bool       real_less_real(double d1, double d2);
extern operator bool       real_greater_real(double d1, double d2);
extern operator bool       real_equal_real(double d1, double d2);

extern method string_t     real_to_string(double d);

extern constant double pi;
#endif
