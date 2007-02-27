#ifndef MATRIX_TR_H
#define MATRIX_TR_H

#include "primtype.h"
#include "object.h"
#include "symbol_tr.h"

typedef struct matrix_s *matrix_tr;
extern primtype_t matrix_type;
extern void matrix_init(void);

extern constructor matrix_tr matrix_new(int cols, int rows);
extern constructor matrix_tr matrix_eye(int n);
extern constructor matrix_tr matrix_rand(int m, int n);
extern constructor matrix_tr matrix_randn(int m, int n);

extern method int       matrix_cols(matrix_tr m);
extern method int       matrix_rows(matrix_tr m);
extern method int       matrix_length(matrix_tr m);
extern method matrix_tr matrix_copy(matrix_tr m);

extern method double    matrix_get(matrix_tr m, int i, int j);
extern method void      matrix_set(matrix_tr m, int i, int j, double v);

extern method double    matrix_getl(matrix_tr m, int idx);
extern method void      matrix_setl(matrix_tr m, int idx, double v);

extern method double    matrix_norm(matrix_tr m);
extern method double    matrix_max(matrix_tr m);
extern method double    matrix_min(matrix_tr m);

extern method void      matrix_scale(matrix_tr a, double b);
extern method matrix_tr matrix_transpose(matrix_tr a);
extern method matrix_tr matrix_sort(matrix_tr a);
extern method matrix_tr matrix_solve(matrix_tr A, matrix_tr x);

extern method matrix_tr matrix_repmat(matrix_tr a, int i, int j);
extern method matrix_tr matrix_reshape(matrix_tr a, int i, int j);

extern operator matrix_tr matrix_minus(matrix_tr a);
extern operator matrix_tr matrix_plus_matrix(matrix_tr a, matrix_tr b);
extern operator matrix_tr matrix_minus_matrix(matrix_tr a, matrix_tr b);
extern operator matrix_tr matrix_times_matrix(matrix_tr a, matrix_tr b);

extern operator matrix_tr matrix_plus_real(matrix_tr a, double d);
extern operator matrix_tr matrix_minus_real(matrix_tr a, double d);
extern operator matrix_tr matrix_times_real(matrix_tr a, double d);

extern operator matrix_tr real_plus_matrix(double d, matrix_tr b);
extern operator matrix_tr real_minus_matrix(double d, matrix_tr b);
extern operator matrix_tr real_times_matrix(double d, matrix_tr b);

extern method matrix_tr matrix_exp(matrix_tr m);
extern method matrix_tr matrix_sin(matrix_tr m);
extern method matrix_tr matrix_sinc(matrix_tr m);

extern void             matrix_map(matrix_tr m, double (*f)(double ));
extern void             matrix_fillfromarray(matrix_tr, double *array);
extern void             matrix_fillwithfunction(matrix_tr, double (*f)());

extern method string_t  matrix_to_string(matrix_tr m);

#ifndef MATRIX_IMPLEMENTATION
#define INLINE extern inline
#include "matrix_inline.c"
#undef INLINE
#endif

#endif
