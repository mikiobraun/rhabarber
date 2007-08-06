#include <stddef.h>
#include <stdio.h>
#include <complex.h>
#include <stdbool.h>

#define evalprint(fmt, exp) printf(#fmt, #exp, exp)

#define FOR(i,n,code) for(int i = 1; i < n; i++) code

int main()
{
  evalprint(%30s = %3d\n, sizeof(bool));
  evalprint(%30s = %3d\n, sizeof(char));
  evalprint(%30s = %3d\n, sizeof(short));
  evalprint(%30s = %3d\n, sizeof(int));
  evalprint(%30s = %3d\n, sizeof(long));
  evalprint(%30s = %3d\n, sizeof(long long));
  printf("\n");
  evalprint(%30s = %3d\n, sizeof(float));
  evalprint(%30s = %3d\n, sizeof(double));
  evalprint(%30s = %3d\n, sizeof(long double));
  evalprint(%30s = %3d\n, sizeof(void*));
  printf("\n");
  evalprint(%30s = %3d\n, sizeof(float complex));
  evalprint(%30s = %3d\n, sizeof(double complex));
  evalprint(%30s = %3d\n, sizeof(long double complex));
  printf("\n");

  evalprint(%30s = %f\n, creal(csqrt(-1)));
  evalprint(%30s = %fI\n, cimag(csqrt(-1)));

  double complex a = 3 + 4.0*_Complex_I;
}
