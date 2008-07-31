
/**************************************************************************
 * Desc: Vector functions
 * Author: Andrew Howard
 * Date: 10 Dec 2002
 * CVS: $Id: pf_vector.c 1713 2003-08-23 04:03:43Z inspectorg $
 *************************************************************************/

#include <math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_linalg.h>

#include "pf_vector.h"


// Return a zero vector
pf_vector_t pf_vector_zero()
{
  pf_vector_t c;

  c.v[0] = 0.0;
  c.v[1] = 0.0;
  c.v[2] = 0.0;
  
  return c;
}


// Check for NAN or INF in any component
int pf_vector_finite(pf_vector_t a)
{
  int i;
  
  for (i = 0; i < 3; i++)
    if (!finite(a.v[i]))
      return 0;
  
  return 1;
}


// Print a vector
void pf_vector_fprintf(pf_vector_t a, FILE *file, const char *fmt)
{
  int i;

  for (i = 0; i < 3; i++)
  {
    fprintf(file, fmt, a.v[i]);
    fprintf(file, " ");
  }
  fprintf(file, "\n");

  return;     
}


// Simple vector addition
pf_vector_t pf_vector_add(pf_vector_t a, pf_vector_t b)
{
  pf_vector_t c;

  c.v[0] = a.v[0] + b.v[0];
  c.v[1] = a.v[1] + b.v[1];
  c.v[2] = a.v[2] + b.v[2];
  
  return c;
}


// Simple vector subtraction
pf_vector_t pf_vector_sub(pf_vector_t a, pf_vector_t b)
{
  pf_vector_t c;

  c.v[0] = a.v[0] - b.v[0];
  c.v[1] = a.v[1] - b.v[1];
  c.v[2] = a.v[2] - b.v[2];
  
  return c;
}


// Transform from local to global coords (a + b)
pf_vector_t pf_vector_coord_add(pf_vector_t a, pf_vector_t b)
{
  pf_vector_t c;

  c.v[0] = b.v[0] + a.v[0] * cos(b.v[2]) - a.v[1] * sin(b.v[2]);
  c.v[1] = b.v[1] + a.v[0] * sin(b.v[2]) + a.v[1] * cos(b.v[2]);
  c.v[2] = b.v[2] + a.v[2];
  c.v[2] = atan2(sin(c.v[2]), cos(c.v[2]));
  
  return c;
}


// Transform from global to local coords (a - b)
pf_vector_t pf_vector_coord_sub(pf_vector_t a, pf_vector_t b)
{
  pf_vector_t c;

  c.v[0] = +(a.v[0] - b.v[0]) * cos(b.v[2]) + (a.v[1] - b.v[1]) * sin(b.v[2]);
  c.v[1] = -(a.v[0] - b.v[0]) * sin(b.v[2]) + (a.v[1] - b.v[1]) * cos(b.v[2]);
  c.v[2] = a.v[2] - b.v[2];
  c.v[2] = atan2(sin(c.v[2]), cos(c.v[2]));
  
  return c;
}


// Return a zero matrix
pf_matrix_t pf_matrix_zero()
{
  int i, j;
  pf_matrix_t c;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      c.m[i][j] = 0.0;
        
  return c;
}


// Check for NAN or INF in any component
int pf_matrix_finite(pf_matrix_t a)
{
  int i, j;
  
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      if (!finite(a.m[i][j]))
        return 0;
  
  return 1;
}


// Print a matrix
void pf_matrix_fprintf(pf_matrix_t a, FILE *file, const char *fmt)
{
  int i, j;

  for (i = 0; i < 3; i++)
  {
    for (j = 0; j < 3; j++)
    {
      fprintf(file, fmt, a.m[i][j]);
      fprintf(file, " ");
    }
    fprintf(file, "\n");
  }
  return;     
}


// Compute the matrix inverse
pf_matrix_t pf_matrix_inverse(pf_matrix_t a, double *det)
{
  double lndet;
  int signum;
  gsl_permutation *p;
  gsl_matrix_view A, Ai;

  pf_matrix_t ai;

  A = gsl_matrix_view_array((double*) a.m, 3, 3);
  Ai = gsl_matrix_view_array((double*) ai.m, 3, 3);
  
  // Do LU decomposition
  p = gsl_permutation_alloc(3);
  gsl_linalg_LU_decomp(&A.matrix, p, &signum);

  // Check for underflow
  lndet = gsl_linalg_LU_lndet(&A.matrix);
  if (lndet < -1000)
  {
    //printf("underflow in matrix inverse lndet = %f", lndet);
    gsl_matrix_set_zero(&Ai.matrix);
  }
  else
  {
    // Compute inverse
    gsl_linalg_LU_invert(&A.matrix, p, &Ai.matrix);
  }

  gsl_permutation_free(p);

  if (det)
    *det = exp(lndet);

  return ai;
}


// Decompose a covariance matrix [a] into a rotation matrix [r] and a diagonal
// matrix [d] such that a = r d r^T.
void pf_matrix_unitary(pf_matrix_t *r, pf_matrix_t *d, pf_matrix_t a)
{
  int i, j;
  gsl_matrix *aa;
  gsl_vector *eval;
  gsl_matrix *evec;
  gsl_eigen_symmv_workspace *w;

  aa = gsl_matrix_alloc(3, 3);
  eval = gsl_vector_alloc(3);
  evec = gsl_matrix_alloc(3, 3);

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      gsl_matrix_set(aa, i, j, a.m[i][j]);

  // Compute eigenvectors/values
  w = gsl_eigen_symmv_alloc(3);
  gsl_eigen_symmv(aa, eval, evec, w);
  gsl_eigen_symmv_free(w);

  *d = pf_matrix_zero();
  for (i = 0; i < 3; i++)
  {
    d->m[i][i] = gsl_vector_get(eval, i);
    for (j = 0; j < 3; j++)
      r->m[i][j] = gsl_matrix_get(evec, i, j);
  }
  
  gsl_matrix_free(evec);
  gsl_vector_free(eval);
  gsl_matrix_free(aa);
  
  return;
}

