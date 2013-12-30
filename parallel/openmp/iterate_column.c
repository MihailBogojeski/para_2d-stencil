#include "common.h"
#define COLUMSPARALLELFLAG
#define ROWSPARALLELFLAG
//#define random_FLAG
static void update(double **primary, double **secondary, int j, int k, double **vectors);

void iterate(double **primary, double **secondary, double **vectors)
{


//Iterations over the whole matrix
for (int i = 0; i < options.iter;  i++){
//Iterations over rows
    #if defined (ROWS_PARALLEL_FLAG)
     #pragma omp parallel
        { 
     #endif
    #if defined(ROWS_PARALLEL_FLAG) 
      #pragma omp for 
    #endif
    for(int k = 0; k < options.m; k++){
        //Iterations over columns
        #if defined(COLUMS_PARALLEL_FLAG) && !defined(ROWS_PARALLEL_FLAG)
	  #pragma omp parallel
          { 
	#endif

        #if defined(COLUMS_PARALLEL_FLAG)
          #pragma omp for 
        #endif
        for(int j = 0; j < options.n; j++){
            update(primary, secondary, j, k, vectors);
        }
        #if defined(COLUMS_PARALLEL_FLAG) && !defined(ROWS_PARALLEL_FLAG)
          }
        #endif
    }  

    #if defined(ROWS_PARALLEL_FLAG) 
      }
    #endif
 
}

   double **temp = primary;
    primary = secondary;
    secondary = temp;

    for (int i = 0; i < options.n; i++){
      for (int j = 0; j < options.m; j++){
        debug("%3.4f ", primary[i][j]);
      }
      debug("\n");
    }
    debug("\n\n");

  
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

