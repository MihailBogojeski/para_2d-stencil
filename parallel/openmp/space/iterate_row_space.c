#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double **primary, double *secondary, int j, int k, double **vectors);

void iterate(double **primary, double **vectors)
{
  double start, finish;
  
  double **tmp_rows = malloc(2 * sizeof(double*));
  if (tmp_rows == NULL){
    bail_out(EXIT_FAILURE, "malloc tmp_rows");
  }
  
  for (int i = 0; i < 2; i++){
    tmp_rows[i] = calloc(options.m, sizeof(double));
    if (tmp_rows[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc tmp_rows[%d]", i);
    }
    memcpy(tmp_rows[i], primary[i], options.m * sizeof(double));
  }

  start = omp_get_wtime();
  for (int i = 0; i < options.iter; i++){
    for(int j = 0; j < options.n; j++){
#pragma omp parallel for schedule(static) 
      for(int k = 0; k < options.m; k++){
        update(primary, tmp_rows[j%2], j, k, vectors);
      }
      if (j > 0) {
        memcpy(primary[j-1],tmp_rows[(j-1)%2], options.m * sizeof(double));
      }
    }
    memcpy(primary[options.n-1],tmp_rows[(options.n-1)%2], options.m * sizeof(double));
  }
  finish = omp_get_wtime();
  
  double usec_diff = finish - start;
  fprintf(stderr,"loop time = %f\n", usec_diff);
  
  for (int i = 0; i < 2; i++){
    free(tmp_rows[i]);
  }
  free(tmp_rows);
}

static void update(double **primary, double *secondary, int j, int k, double **vectors){
  debug("update %d %d\n", j, k);

  double sum = 0;

  if (k-1 < 0) {
    sum += vectors[3][j];
  }
  else{
    sum += primary[j][k-1];
  }

  if (j-1 < 0) {
    sum += vectors[0][k];
  }
  else{
    sum += primary[j-1][k];
  }

  if (k+1 >= options.m) {
    sum += vectors[1][j];
  }
  else{
    sum += primary[j][k+1];
  }

  if (j+1 >= options.n) {
    sum += vectors[2][k];
  }
  else{
    sum += primary[j+1][k];
  }

  secondary[k] = sum/(double)4;
}

