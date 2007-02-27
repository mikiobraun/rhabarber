
#ifndef MATRIX_IMPLEMENTATION
struct matrix_s
{
  RHA_OBJECT;
  int rows;
  int cols;
  int length;
  double *array;
};
#endif

#if defined(MATRIX_IMPLEMENTATION) || defined(USE_INLINES)

#ifndef CHECK
#define CHECK(x) CHECK_TYPE(matrix, x)
#define UNDEF_CHECK
#endif

/*
 *
 * Matrix dimensions
 *
 */

INLINE
int matrix_rows(matrix_tr m)
{
  CHECK(m);
  return m->rows;
}

INLINE
int matrix_cols(matrix_tr m)
{
  CHECK(m);
  return m->cols;
}


INLINE
int matrix_length(matrix_tr m)
{
  CHECK(m);
  return m->length;
}

/*
 *
 * Element access
 *
 */

INLINE
double matrix_getl(matrix_tr m, int i)
{
  CHECK(m);
  return m->array[i];
}

INLINE
void matrix_setl(matrix_tr m, int i, double v)
{
  CHECK(m);
  m->array[i] = v;
}


INLINE
double matrix_get(matrix_tr m, int i, int j)
{
  CHECK(m);
  int c = i + j*(m->rows);
  return m->array[c];
}

INLINE
void matrix_set(matrix_tr m, int i, int j, double v)
{
  CHECK(m);
  int c = i + j*(m->rows);
  m->array[c] = v;
}

#ifdef UNDEF_CHECK
#undef CHECK
#endif

#endif
