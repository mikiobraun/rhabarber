#ifndef COMPLEX_TR_H
#define COMPLEX_TR_H

#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"

typedef struct complex_s *complex_tr;
extern primtype_t complex_type;
extern void complex_init();

extern constructor complex_tr complex_new(double real, double imag);
extern constructor complex_tr complex_from_real(double d);

extern method double          complex_real(complex_tr c);
extern method double          complex_imag(complex_tr c);

extern operator complex_tr    complex_minus(complex_tr c);
extern operator complex_tr    complex_plus_complex(complex_tr c1, complex_tr c2);
extern operator complex_tr    complex_minus_complex(complex_tr c1, complex_tr c2);
extern operator complex_tr    complex_times_complex(complex_tr c1, complex_tr c2);
extern operator complex_tr    complex_divide_complex(complex_tr c1, complex_tr c2);

extern operator bool          complex_equal_complex(complex_tr c1, complex_tr c2);

extern method string_t        complex_to_string(complex_tr c);

#endif
