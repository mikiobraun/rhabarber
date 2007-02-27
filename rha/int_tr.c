#include "int_tr.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "alloc.h"
#include "object.h"
#include "utils.h"
#include "primtype.h"
#include "symbol_tr.h"
#include "string_tr.h"
#include "messages.h"

struct int_s
{
  RHA_OBJECT;
  int value;
};

#define CHECK(x) CHECK_TYPE(int, x)

primtype_t int_type = 0;

void int_init(void)
{
  if (!int_type) {
    int_type = primtype_new("int", sizeof(struct int_s));
    (void) int_new(0);
  }
}


int_tr int_new(int value)
{
  int_tr i = object_new(int_type);
  i->value = value;
  return i;
}


int_tr int_copy(int_tr i)
{
  return int_new(int_get(i));
}


int int_get(int_tr i)
{
  CHECK(i);
  return i->value;
}


int int_inc(int i)
{
  return i+1;
}


int int_dec(int i)
{
  return i-1;
}


int int_plus_int(int i1, int i2)
{
  return i1+i2;
}


int int_minus_int(int i1, int i2)
{
  return i1-i2;
}


int int_times_int(int i1, int i2)
{
  return i1*i2;
}


int int_divide_int(int i1, int i2)
{
  if (i2 == 0) rha_error("Division by zero.\n");
  return (int) i1/i2;
}


bool int_less_int(int i1, int i2)
{
  return i1<i2;
}

bool int_greater_int(int i1, int i2)
{
  return i1>i2;
}

bool int_equal_int(int i1, int i2)
{
  return i1 == i2;
}


double int_exp(int i)
{
  return exp((double) i);
}


double int_sin(int i)
{
  return sin((double) i);
}


double int_sinc(int i)
{
  if (i==0) return 1.0;
  else return (sin((double) i)/((double) i));
}


string_t int_to_string(int i)
{
  return sprint("%d", i);
}


int int_minus(int i)
{
  return 0-i;
}


#undef CHECK
