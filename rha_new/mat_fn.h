#ifndef MAT_FN_H
#define MAT_FN_H

#include "rha_types.h"

extern _rha_ mat_t  mat_new(int_t rows, int_t cols);
extern _rha_ mat_t  mat_eye(int_t n);
extern _rha_ mat_t  mat_rand(int_t m, int_t n);
extern _rha_ mat_t  mat_randn(int_t m, int_t n);

extern _rha_ int_t  mat_cols(mat_t m);
extern _rha_ int_t  mat_rows(mat_t m);
extern _rha_ int_t  mat_len(mat_t m);
extern _rha_ mat_t  mat_copy(mat_t m);

extern _rha_ real_t mat_get(mat_t m, int_t i, int_t j);
extern _rha_ void   mat_set(mat_t m, int_t i, int_t j, real_t v);

extern _rha_ real_t mat_getl(mat_t m, int_t idx);
extern _rha_ void   mat_setl(mat_t m, int_t idx, real_t v);

extern _rha_ real_t mat_norm(mat_t m);
extern _rha_ real_t mat_max(mat_t m);
extern _rha_ real_t mat_min(mat_t m);

extern _rha_ void   mat_scale(mat_t a, real_t b);
extern _rha_ mat_t  mat_transpose(mat_t a);
extern _rha_ mat_t  mat_sort(mat_t a);
extern _rha_ mat_t  mat_solve(mat_t A, mat_t x);

extern _rha_ mat_t  mat_repmat(mat_t a, int_t i, int_t j);
extern _rha_ mat_t  mat_reshape(mat_t a, int_t i, int_t j);

extern _rha_ mat_t  mat_minus(mat_t a);
extern _rha_ mat_t  mat_plus_mat(mat_t a, mat_t b);
extern _rha_ mat_t  mat_minus_mat(mat_t a, mat_t b);
extern _rha_ mat_t  mat_times_mat(mat_t a, mat_t b);
extern _rha_ mat_t  mat_dottimes_mat(mat_t a, mat_t b);
extern _rha_ mat_t  mat_dotdivide_mat(mat_t a, mat_t b);

extern _rha_ mat_t  mat_plus_real(mat_t a, real_t d);
extern _rha_ mat_t  mat_minus_real(mat_t a, real_t d);
extern _rha_ mat_t  mat_times_real(mat_t a, real_t d);
extern _rha_ mat_t  mat_divide_real(mat_t a, real_t d);

extern _rha_ mat_t  real_plus_mat(real_t d, mat_t b);
extern _rha_ mat_t  real_minus_mat(real_t d, mat_t b);
extern _rha_ mat_t  real_times_mat(real_t d, mat_t b);

extern _rha_ mat_t  mat_exp(mat_t m);
extern _rha_ mat_t  mat_sin(mat_t m);
extern _rha_ mat_t  mat_sinc(mat_t m);

extern       void   mat_map(mat_t m, real_t (*f)(real_t ));
extern       void   mat_fillfromarray(mat_t, real_t *array);
extern       void   mat_fillwithfunction(mat_t, real_t (*f)());

extern _rha_ string_t mat_to_string(mat_t m);

#endif
