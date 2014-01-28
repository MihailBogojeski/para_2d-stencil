#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double **primary, double **secondary, int j, int k);
static void swap (double ***primary, double ***secondary);

void iterate(double **primary, double **vectors)
{
  double start, finish;
  
  double **secondary = malloc((options.n + 2) * sizeof(double*));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }
  for (int i = 0; i < options.n + 2; i++){
    secondary[i] = calloc(options.m + 2, sizeof(double));
    if (secondary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc secondary[%d]", i);
    }
  }
  for (int i = 0; i < options.n + 2; i++){
    memcpy(secondary[i], primary[i], (options.m + 2) * sizeof(double));
  }

  start = omp_get_wtime();
  for (int i = 0; i < options.iter; i++){
    for(int j = 1; j <= options.n; j++){
      for(int k = 1; k <= options.m; k++){
        update(primary, secondary, j, k);
      }
    }
    swap(&primary, &secondary);
  }
  finish = omp_get_wtime();
  
  double usec_diff = finish - start;
  fprintf(stderr,"loop time = %f\n", usec_diff);
  
  if (options.iter % 2 == 1){
    swap(&primary, &secondary);
    for (int i = 0; i <= options.n; i++){
      memcpy(primary[i],secondary[i],(options.m + 2) * sizeof(double));
    } 
  }
  
  for (int i = 0; i < options.n + 2; i++){
    free(secondary[i]);
  }
  free(secondary);
}

static void update(double **primary, double **secondary, int j, int k){
  debug("update %d %d\n", j, k);

  double sum = 0;

  sum += primary[j][k-1];
  sum += primary[j-1][k];
  sum += primary[j][k+1];
  sum += primary[j+1][k];

  secondary[j][k] = sum/(double)4;
}

static void swap (double ***primary, double ***secondary){
  double **temp = *primary;
  *primary = *secondary;
  *secondary = temp;
  /*
  for (int i = 0; i < options.n; i++){
    memcpy((*primary)[i],(*secondary)[i],options.m * sizeof(double));
  }
  */
  
}
