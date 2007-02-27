#include "real_tr.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "alloc.h"
#include "builtin_tr.h"
#include "core.h"
#include "eval.h"
#include "utils.h"
#include "int_tr.h"
#include "complex_tr.h"
#include "string_tr.h"
#include "symbol_tr.h"

struct real_s
{
  RHA_OBJECT;
  double value;
};

#define CHECK(x) CHECK_TYPE(real, x)

primtype_t real_type = 0;

void real_init(void)
{
  if (!real_type) {
    real_type = primtype_new("real", sizeof(struct real_s));
    (void) real_new(0);
  }
}


real_tr real_new(double value)
{
  real_tr r = object_new(real_type);
  r->value = value;
  return r;
}


real_tr real_copy(real_tr r)
{
  return real_new(real_get(r));
}


double real_get(real_tr r)
{
  CHECK(r);
  return r->value;
}


double real_minus(double d)
{
  return 0.0 - d;
}


double real_plus_real(double d1, double d2)
{
  return d1 + d2;
}


double real_minus_real(double d1, double d2)
{
  return d1 - d2;
}


double real_times_real(double d1, double d2)
{
  return d1 * d2;
}


double real_divide_real(double d1, double d2)
{
  return (double) d1 / d2;
}


bool real_less_real(double d1, double d2)
{
  return d1 < d2;
}


bool real_greater_real(double d1, double d2)
{
  return d1 > d2;
}


bool real_equal_real(double d1, double d2)
{
  return d1 == d2;
}


double real_from_int(int i)
{
  return 1.0 * i;
}


object_t real_sqrt(double d)
{
  if(d >= 0) 
    return real_new(sqrt(d));
  else
    return complex_new(0, -sqrt(-d));
}


double real_exp(double d)
{
  return exp(d);
}


double real_sin(double d)
{
  return sin(d);
}


double real_sinc(double d)
{
  if (d==0) return 1.0;
  else return (sin(d)/d);
}


string_t real_to_string(double d)
{
  return sprint("%f", d);
}


double pi = 3.1415926536;

#undef CHECK
