#include <stdio.h>
#include <stdlib.h>
#include "alloc.h"
#include "matrix_tr.h"


void printmatrix(matrix_tr m)
{
  for(int i = 0; i < matrix_cols(m); i++) {
    for(int j = 0; j < matrix_rows(m); j++)
      printf("%+3.2f  ", matrix_get(m, i, j));
    printf("\n");
  }
  printf("\n");
}

void matrixmult(matrix_tr c, matrix_tr a, matrix_tr b)
{
  for(int i = 0; i < matrix_length(c); i++)
      matrix_setl(c, i, 0);

  for(int i = 0; i < matrix_cols(a); i++)
    for(int j = 0; j < matrix_rows(b); j++) 
      for(int k = 0; k < matrix_cols(a); k++)
	matrix_set(c, i, j, 
		   matrix_get(c, i, j) +
		   matrix_get(a, i, k)
		   * matrix_get(b, k, j));
}


int main(int argc, char **argv)
{
  double marray[] = { 1, 2, 3, 4,  5, 6, 7, 8,  9, 10, 11, 12 };
  double narray[] = { 1, 0, 0, 2, 0, 0 };
  double xarray[] = { 1, 2, 3 };

  GC_init();

  matrix_tr m = matrix_new(3, 4); matrix_fillfromarray(m, marray);
  matrix_tr n = matrix_new(2, 3); matrix_fillfromarray(n, narray);
  matrix_tr x = matrix_new(3, 1); matrix_fillfromarray(x, xarray);
  printmatrix(n);
  printmatrix(x);
  printmatrix(matrix_mult(n, x));
  printf("------------------------------------------------------------\n");
  printmatrix(n);
  printmatrix(m);
  printmatrix(matrix_mult(n, m));
  printmatrix(matrix_add(m, m));
  
  /*
  matrix_tr A = matrix_new(2000, 2000);
  matrix_tr B = matrix_new(2000, 2000);
  matrix_fillwithfunction(A, drand);

  matrix_tr C = matrix_mult(A,B);*/

  {
    double Adat[] = { 2, 1, 0,  0, 5, 0, 3, 2, 1 };
    double xdat[] = { 18, 16, 4 };
    
    matrix_tr A = matrix_new(3,3); matrix_fillfromarray(A, Adat);
    matrix_tr x = matrix_new(3,1); matrix_fillfromarray(x, xdat);

    matrix_tr y = matrix_solve(A, x);
    printmatrix(y);
  }
  {
    matrix_tr A = matrix_rand(1500, 1500);
    matrix_solve(A, A);
  }
}
