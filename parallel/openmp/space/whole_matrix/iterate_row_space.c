#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double **primary, double *secondary, int j, int k);

void iterate(double ***primary)
{
  double start, finish;
  
  double **tmp_rows = malloc(2 * sizeof(double*));
  if (tmp_rows == NULL){
    bail_out(EXIT_FAILURE, "malloc tmp_rows");
  }
  
  for (int i = 0; i < 2; i++){
    tmp_rows[i] = calloc(COL_VEC, sizeof(double));
    if (tmp_rows[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc tmp_rows[%d]", i);
    }
    memcpy(tmp_rows[i], (*primary)[i+1], (COL_VEC) * sizeof(double));
  }

  start = omp_get_wtime();
  for (int i = 0; i < options.iter; i++){
    for(int j = 1; j <= options.n; j++){
#pragma omp parallel for schedule (static)
      for(int k = 1; k <= options.m; k++){
        update(*primary, tmp_rows[(j+1)%2], j, k);
      }
      if (j > 1) {
        memcpy(&((*primary)[j-1][1]),&tmp_rows[(j)%2][1], options.m * sizeof(double));
      }
    }
    memcpy(&((*primary)[options.n][1]),&tmp_rows[(options.n+1)%2][1], options.m * sizeof(double));
  }
  finish = omp_get_wtime();
  
  double usec_diff = finish - start;
  fprintf(stderr,"loop time = %f\n", usec_diff);
  
  for (int i = 0; i < 2; i++){
    free(tmp_rows[i]);
  }
  free(tmp_rows);
}

static void update(double **primary, double *secondary, int j, int k){
  debug("update %d %d\n", j, k);

  double sum = 0;
  
  sum += primary[j][k-1];
  sum += primary[j-1][k];
  sum += primary[j][k+1];
  sum += primary[j+1][k];

  secondary[k] = sum/(double)4;
}

