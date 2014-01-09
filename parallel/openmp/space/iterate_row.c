#include "common.h"
#include <omp.h>

static void update(double **primary, double **secondary, int j, int k, double **vectors);

void iterate(double **primary, double **vectors)
{
  double **secondary = malloc(options.n * sizeof(double*));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }
  for (int i = 0; i < options.n; i++){
    secondary[i] = calloc(options.m, sizeof(double));
    if (secondary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc secondary[%d]", i);
    }
  }

  for (int i = 0; i < options.iter; i++){
    for(int j = 0; j < options.n; j++){
      #pragma omp parallel
      {
        #pragma omp for nowait
        for(int k = 0; k < options.m; k++){
          update(primary, secondary, j, k, vectors);
        }
      }
    }
    double **temp = primary;
    primary = secondary;
    secondary = temp;
    /*
       for (int i = 0; i < options.n; i++){
       for (int j = 0; j < options.m; j++){
       debug("%3.4f ", primary[i][j]);
       }
       debug("\n");
       }
       debug("\n\n");
       */
  }
  for (int i = 0; i < options.n; i++){
    free(secondary[i]);
  }
  free(secondary);
}
static void update(double **primary, double **secondary, int j, int k, double **vectors){
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

  secondary[j][k] = sum/(double)4;
}

