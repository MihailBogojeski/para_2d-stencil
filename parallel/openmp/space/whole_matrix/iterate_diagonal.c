#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double **primary, double **secondary, int j, int k);
static void swap (double ***primary, double ***secondary);

void iterate(double ***primary){
  double start, finish;
  if (options.nproc != 0){
    omp_set_num_threads(options.nproc);
  }
  
  double **secondary = malloc((ROW_VEC) * sizeof(double*));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }
  for (int i = 0; i < ROW_VEC; i++){
    secondary[i] = calloc(COL_VEC, sizeof(double));
    if (secondary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc secondary[%d]", i);
    }
  }
  for (int i = 0; i < ROW_VEC; i++){
    memcpy(secondary[i], (*primary)[i], (COL_VEC) * sizeof(double));
  }

  start = omp_get_wtime();


  for (int i = 0; i < options.iter; i++)
  {
    //Iterates over the left half 
    #pragma omp parallel for schedule(static)
    for ( int x = 1; x <= options.m; x++)
    {
       for(int y = 1, offset_x = x; y <= options.n && offset_x >= 1; offset_x--, y++)
       {
         #ifdef DIAG_DEBUG 
	 printf("RIGHT: i: %d  y: %d x: %d offset_y: %d \n", i,y, x, offset_x);
         fflush(stdout);
         #endif
	 update(*primary, secondary,y, offset_x); 
	
	}
    }
       #ifdef DIAG_DEBUG 
       printf("=================================================\n");
       fflush(stdout);
       #endif
    //Iterates over the right half 
    #pragma omp parallel for schedule(static)
    for(int y = 1; y <= options.n ;y++ )
    {
	for(int x = options.m, offset_y = y; x >= 1 && offset_y <= options.n; offset_y++, x--)
	{
         #ifdef DIAG_DEBUG 
	 printf("LEFT: i: %d  y: %d x: %d offset_y: %d \n", i,y, x, offset_y);
         fflush(stdout);
         #endif
	 update(*primary, secondary, offset_y, x); 
	}	
    }
    swap(primary,&secondary);
  }
  finish = omp_get_wtime();
  
  double usec_diff = finish - start;
  fprintf(stderr,"loop time = %f\n", usec_diff);
  
  
  for (int i = 0; i < ROW_VEC; i++){
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
