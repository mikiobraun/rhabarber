#include "complex_tr.h"

#include <stdio.h>
#include <assert.h>

#include "alloc.h"
#include "utils.h"
#include "object.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"

#include "real_tr.h"

struct complex_s
{
  RHA_OBJECT;
  double real;
  double imag;
};

#define CHECK(x) CHECK_TYPE(complex, x)

primtype_t complex_type = 0;

void complex_init(void)
{
  if (!complex_type) {
    complex_type = primtype_new("complex", sizeof(struct complex_s));
    (void) complex_new(0, 0);
  }
}


complex_tr complex_new(double real, double imag)
{
  complex_tr c = object_new(complex_type);
  c->real = real;
  c->imag = imag;
  return c;
}


double complex_real(complex_tr c)
{
  CHECK(c);
  return c->real;
}


double complex_imag(complex_tr c)
{
  CHECK(c);
  return c->imag;  
}


complex_tr complex_minus(complex_tr a)
{
  return complex_new(0.0 - a->real, 0.0 - a->imag);
}


complex_tr complex_plus_complex(complex_tr a, complex_tr b)
{
  CHECK(a);
  CHECK(b);
  return complex_new(a->real + b->real, a->imag + b->imag);
}


complex_tr complex_minus_complex(complex_tr a, complex_tr b)
{
  CHECK(a);
  CHECK(b);
  return complex_new(a->real - b->real, a->imag - b->imag);
}


complex_tr complex_times_complex(complex_tr a, complex_tr b)
{
  CHECK(a);
  CHECK(b);
  return complex_new(a->real * b->real - a->imag * b->imag,
		     a->real * b->imag + a->imag * b->real);
}


complex_tr complex_divide_complex(complex_tr a, complex_tr b)
{
  CHECK(a);
  CHECK(b);
  double bn = b->real*b->real + b->imag*b->imag;
  double bir = b->real / bn;
  double bii = -b->imag / bn;
  return complex_new(a->real * bir - a->imag * bii,
		     a->real * bii + a->imag * bir);
}


complex_tr complex_from_real(double d)
{
  return complex_new(d, 0.0);
}


bool complex_equal_complex(complex_tr c1, complex_tr c2)
{
  return (c1->real == c2->real) && (c1->imag == c2->imag);
}


string_t complex_to_string(complex_tr c)
{
  CHECK(c);
  if(c->imag >= 0) {
    return sprint("%f + %fi", c->real, c->imag);
  }
  else {
    return sprint("%f - %fi", c->real, c->imag);
  }
}

