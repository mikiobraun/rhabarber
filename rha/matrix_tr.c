#include "matrix_tr.h"

// for Stefan's laptop there is the SIMPLE choice which switches blas
// stuff on and off

#include <assert.h>

#ifndef SIMPLE
#include <cblas.h>
#include <clapack.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "core.h"
#include "eval.h"
#include "utils.h"
#include "messages.h"

#include "builtin_tr.h"
#include "function_tr.h"
#include "int_tr.h"
#include "object.h"
#include "real_tr.h"
#include "string_tr.h"
#include "symbol_tr.h"

int lessthan_double(const void *a, const void *b) { 
  double *ap = (double *) a;
  double *bp = (double *) b;
  return *ap < *bp; 
}

#define MATRIX_IMPLEMENTATION
#define INLINE inline
#include "matrix_inline.c"

#define CHECK(m) CHECK_TYPE(matrix, m)

primtype_t matrix_type = 0;

void matrix_init(void)
{
  if (!matrix_type) {
    matrix_type = primtype_new("matrix", sizeof(struct matrix_s));
    (void) matrix_new(0, 0);
  }
}


matrix_tr matrix_new(int r, int c)
{
  matrix_tr m = object_new(matrix_type);
  m->rows = r;
  m->cols = c;
  m->length = r * c;
  m->array = ALLOC_RAW(sizeof(*m->array) * m->length);
  return m;
}


void matrix_fillfromarray(matrix_tr m, double *array)
{
  CHECK(m);
  memcpy(m->array, array, sizeof(*m->array) * m->length);
}


void matrix_fillwithfunction(matrix_tr m, double (*f)())
{
  CHECK(m);
  for(int i = 0; i < m->length; i++) {
    m->array[i] = (*f)();
  }
}


matrix_tr matrix_copy(matrix_tr m)
{
  CHECK(m);
  matrix_tr a = matrix_new(m->rows, m->cols);
  memcpy(a->array, m->array, sizeof(*m->array) * m->length);
  return a;
}


matrix_tr matrix_eye(int n)
{
  matrix_tr I = matrix_new(n, n);
  for(int i = 0; i < n; i++)
    for(int j = 0; j < n; j++)
      matrix_set(I, i, j, 0);
  for(int i = 0; i < n; i++)
    matrix_set(I, i, i, 1);
  return I;
}


matrix_tr matrix_transpose(matrix_tr a)
{
  CHECK(a);
  matrix_tr b = matrix_new(a->cols, a->rows);
  for(int i = 0; i < a->rows; i++)
    for(int j = 0; j < a->cols; j++) 
      matrix_set(b, j, i, matrix_get(a, i, j));
  return b;
}


double matrix_norm(matrix_tr a)
{
  // Frobenius norm
#ifndef SIMPLE
  return cblas_ddot(a->length, a->array, 1, a->array, 1);
#else
  return 0;
#endif
}


static double drand()
{
  return ((double)rand())/RAND_MAX;
}


static double drandn()
{
  double pi = 3.1415926536;
  double r, phi;
  while (!(r = drand())) ;    // r must be != 0
  while (!(phi = drand())) ;  // phi must be != 0
  return cos(2*pi*phi)*sqrt(-2*log(r));
}


matrix_tr matrix_rand(int m, int n)
{
  matrix_tr X = matrix_new(m, n);
  matrix_fillwithfunction(X, drand);
  return X;
}


matrix_tr matrix_randn(int m, int n)
{
  matrix_tr X = matrix_new(m, n);
  matrix_fillwithfunction(X, drandn);
  return X;
}


void matrix_scale(matrix_tr a, double b)
{
  CHECK(a);
#ifndef SIMPLE
  cblas_dscal(a->length, b, a->array, 1);
#endif 
}


double matrix_max(matrix_tr m)
{
  CHECK(m);
  double cand = m->array[0];
  for (int i = 1; i < m->length; i++) {
    double dum = m->array[i];
    cand = (cand<dum) ? dum : cand;
  }
  return cand;
}


double matrix_min(matrix_tr m)
{
  CHECK(m);
  double cand = m->array[0];
  for (int i = 1; i < m->length; i++) {
    double dum = m->array[i];
    cand = (cand>dum) ? dum : cand;
  }
  return cand;
}


matrix_tr matrix_solve(matrix_tr A, matrix_tr B)
{
  CHECK(A);
  CHECK(B);
  matrix_tr AA = matrix_copy(A); //mikio not sure about this!
  matrix_tr C = matrix_copy(B);
  int *P = malloc(sizeof(int) * AA->rows);
#ifdef DONTUSETHISFUNCTIONFORNOW
  clapack_dgesv(CblasColMajor, 
		AA->rows, C->cols,
		AA->array, AA->rows, 
		P,
		C->array, C->rows);
#endif
  free(P);		
  return C;
}


matrix_tr matrix_sort(matrix_tr A)
{
  CHECK(A);
  extern int lessthan_double();
  matrix_tr B = matrix_copy(A);
  //qsort(B->array, B->length, sizeof(double), lessthan_double);
  for(int i = 0; i < B->length; i++)
    for(int j = 0; j < B->length; j++) {
      if(B->array[i] > B->array[j]) {
	double swap = B->array[i];
	B->array[i] = B->array[j];
	B->array[j] = swap;
      }
    }
  return B;
}


void matrix_map(matrix_tr m, double (*f)(double ))
{
  CHECK(m);
  for(int i = 0; i < m->length; i++) {
    m->array[i] = (*f)(m->array[i]);
  }
}


void copy_col(matrix_tr a, matrix_tr u, int j)
{
  for (int i = 0; i < a->rows; i++) {
    double aij = matrix_get(a, i, j);
    u->array[i] = aij;
  }
}


real_tr real_from_matrix(matrix_tr m) {
  if (m->rows==1 && m->cols==1) {
    return real_new(m->array[0]);
  }
  else {
    rha_error("matrix.to_real can only convert 1x1 matrices.\n");
    return 0;
  }
}


BUILTIN(b_matrix_map)
{
  char *msg = "map(function, matrix, matrix);\n// matrices must have same number of rows.\n// function must return real.";
  CHECK_TYPE(tuple, in);
  if (tuple_length(in)!=4) { printf("1");
    printusage(msg);
    return 0;
  }
  function_tr f = eval(env, tuple_get(in, 1));
  if (!f) return 0;
  matrix_tr a = eval(env, tuple_get(in, 2));
  if (!a) return 0;
  matrix_tr b = eval(env, tuple_get(in, 3));
  if (!b) return 0;
  if (!rha_callable(f) || 
      !HAS_TYPE(matrix, a) || 
      !HAS_TYPE(matrix, b)) {
    printf("2");printusage(msg);
    return 0;
  }
  if (a->rows!=b->rows) { printf("3"); printusage(msg); return 0; }
  object_t cll = tuple_new(3);
  matrix_tr u = matrix_new(a->rows, 1);
  matrix_tr v = matrix_new(b->rows, 1);
  tuple_set(cll, 0, f);
  tuple_set(cll, 1, u);
  tuple_set(cll, 2, v);
  matrix_tr c = matrix_new(a->cols, b->cols);
  for (int i = 0; i < a->cols; i++) {
    //matrix_fillfromarray(u, &(a->array[i * (a->rows)]));
    copy_col(a, u, i);
    for (int j = 0; j < b->cols; j++) {
      //matrix_fillfromarray(v, &(b->array[j * (b->rows)]));
      copy_col(b, v, j);
      object_t r = eval(env, cll);
      if (!r)  { printf("4");printusage(msg); return 0; }
      if (a->rows==1 && HAS_TYPE(matrix, r)) r = real_from_matrix(r);
      else if (!HAS_TYPE(real, r)) { printf("5");printusage(msg); return 0; }
      matrix_set(c, i, j, sqrt(real_get(r)));
    }
  }
  return c;
}


matrix_tr matrix_repmat(matrix_tr a, int r, int c)
{
  CHECK(a);
  // allocate memory for the result
  matrix_tr res = matrix_new(r*(a->rows), c*(a->cols));
  int len = sizeof(*a->array) * a->rows;  // copy column by column
  for (int k=0; k<a->cols; k++) {
    double *inspot = &(a->array[k*(a->rows)]);
    for (int i=0; i<r; i++)
      for (int j=0; j<c; j++) {
	double *outspot = &(res->array[i*(a->rows) + j*(a->cols)*(a->rows)*r + k*(a->rows)*r]);
	memcpy(outspot, inspot, len);
      }
  }
  return res;
}


matrix_tr matrix_reshape(matrix_tr a, int r, int c)
{
  CHECK(a);
  // does the new size match the matrix
  if (a->length != r*c) { return 0; }
  // change the shape
  a->rows = r;
  a->cols = c;
  return a;
}


matrix_tr matrix_minus(matrix_tr a)
{
  CHECK(a);
  matrix_tr m = matrix_copy(a);
  for (int i=0; i<m->length; i++) m->array[i] = -m->array[i];
  return m;
}


matrix_tr matrix_plus_matrix(matrix_tr a, matrix_tr b)
{
  CHECK(a);
  CHECK(b);
  // checks dims
  if (a->cols!=b->cols || a->rows!=b->rows) return 0;
  matrix_tr c = matrix_copy(a);
#ifndef SIMPLE
  cblas_daxpy(b->length, 1.0, b->array, 1, c->array, 1);
#endif
  return c;
}


matrix_tr matrix_plus_real(matrix_tr a, double r)
{
  CHECK(a);
  matrix_tr m = matrix_copy(a);
  for (int i=0; i<m->length; i++) m->array[i] = m->array[i] + r;
  return m;
}


matrix_tr real_plus_matrix(double r, matrix_tr a)
{
  return matrix_plus_real(a, r);
}


matrix_tr matrix_minus_matrix(matrix_tr a, matrix_tr b)
{
  CHECK(a);
  CHECK(b);
  // checks dims
  if (a->cols!=b->cols || a->rows!=b->rows) return 0;
  matrix_tr c = matrix_copy(a);
#ifndef SIMPLE
  cblas_daxpy(b->length, -1.0, b->array, 1, c->array, 1);
#endif
  return c;
}


matrix_tr matrix_minus_real(matrix_tr a, double r)
{
  CHECK(a);
  matrix_tr m = matrix_copy(a);
  for (int i=0; i<m->length; i++) m->array[i] = m->array[i] - r;
  return m;
}


matrix_tr real_minus_matrix(double r, matrix_tr a)
{
  CHECK(a);
  matrix_tr m = matrix_copy(a);
  for (int i=0; i<m->length; i++) m->array[i] = r - m->array[i];
  return m;
}


matrix_tr matrix_times_matrix(matrix_tr a, matrix_tr b)
{
  CHECK(a);
  CHECK(b);
  // check dims
  if (a->cols!=b->rows) return 0;
  matrix_tr c = matrix_new(a->rows, b->cols);
  if(b->cols == 1) {
    // matrix-vector multiplication
#ifndef SIMPLE
    cblas_dgemv(CblasColMajor, CblasNoTrans,
		a->rows, a->cols, 
		1.0, a->array, a->rows,
		b->array, 1,
		0.0, c->array, 1);
#endif
  }
  else {
    // matrix-matrix multiplication
#ifndef SIMPLE
    cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
		c->rows, c->cols, a->cols,
		1.0, a->array, a->rows,
		b->array, b->rows,
		0.0, c->array, c->rows);
#endif
  }
  return c;
}


matrix_tr matrix_times_real(matrix_tr a, double r)
{
  CHECK(a);
  matrix_tr res = matrix_copy(a);
  matrix_scale(res, r);
  return res;
}


matrix_tr real_times_matrix(double r, matrix_tr a)
{
  return matrix_times_real(a, r);
}


matrix_tr matrix_exp(matrix_tr a)
     // component-wise exponential
{
  CHECK(a);
  matrix_tr r = matrix_new(a->rows, a->cols);
  for (int i = 0; i < matrix_length(a); i++) 
    matrix_setl(r, i, exp(matrix_getl(a, i)));
  return r;
}


matrix_tr matrix_sin(matrix_tr a)
     // component-wise sin
{
  CHECK(a);
  matrix_tr r = matrix_new(a->rows, a->cols);
  for (int i = 0; i < matrix_length(a); i++) 
    matrix_setl(r, i, sin(matrix_getl(a, i)));
  return r;
}


matrix_tr matrix_sinc(matrix_tr a)
     // component-wise sinc
{
  CHECK(a);
  matrix_tr r = matrix_new(a->rows, a->cols);
  for (int i = 0; i < matrix_length(a); i++) {
    double ddd = matrix_getl(a, i);
    if (ddd==0) matrix_setl(r, i, 1);
    else matrix_setl(r, i, sin(ddd)/ddd);
  }
  return r;
}


string_t matrix_to_string(matrix_tr m)
{
  string_t s = gc_strdup("");
  for(int i = 0; i < m->rows; i++) {
    for(int j = 0; j < m->cols; j++) {
      if (j) s = string_plus_string(s, "  ");
      s = string_plus_string(s, sprint("%5.2f", matrix_get(m, i, j)));
    }
    s = string_plus_string(s, "\n");
  }
  return s;
}

