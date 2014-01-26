#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double *primary, double *secondary, int j, int k, double **vectors);
static void swap (double **primary, double **secondary);

void iterate(double *sub_matrix, double **sub_vecs)
{
  //double start, finish;
  int p, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  double *secondary = calloc(sub_rows * sub_cols, sizeof(double));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }


  MPI_Datatype col, col2; 
  MPI_Type_vector(sub_rows,1,sub_cols,MPI_DOUBLE, &col2); 
  MPI_Type_create_resized(col2, 0, sizeof(double), &col);
  MPI_Type_commit(&col); 


  if (coords[0] > 0){
    int tmp_coords[2] = {coords[0] - 1, coords[1]};
    int tmp_rank = 0;
    MPI_Status status;
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
    MPI_Sendrecv (sub_matrix, sub_cols, MPI_DOUBLE, tmp_rank, 2,
        sub_vecs[0], sub_cols, MPI_DOUBLE, tmp_rank, 0, MPI_COMM_WORLD, &status);
  }
  
  if (coords[0] < dims[0] - 1){
    int tmp_coords[2] = {coords[0] + 1, coords[1]};
    int tmp_rank = 0;
    MPI_Status status;
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
    MPI_Sendrecv (&sub_matrix[sub_cols * (sub_rows - 1)], sub_cols, MPI_DOUBLE, tmp_rank, 0,
        sub_vecs[2], sub_cols, MPI_DOUBLE, tmp_rank, 2, MPI_COMM_WORLD, &status);
  }
  
  if (coords[1] > 0){
    int tmp_coords[2] = {coords[0], coords[1] - 1};
    int tmp_rank = 0;
    MPI_Status status;
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
    MPI_Sendrecv (sub_matrix, 1, col, tmp_rank, 1,
        sub_vecs[3], sub_rows, MPI_DOUBLE, tmp_rank, 3, MPI_COMM_WORLD, &status);
  }
  
  if (coords[1] < dims[1] - 1){
    int tmp_coords[2] = {coords[0], coords[1] + 1};
    int tmp_rank = 0;
    MPI_Status status;
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
    MPI_Sendrecv (&sub_matrix[sub_cols - 1], 1, col, tmp_rank, 3,
        sub_vecs[1], sub_rows, MPI_DOUBLE, tmp_rank, 1, MPI_COMM_WORLD, &status);
  }


  for (int proc=0; proc<p; proc++) {
    if (proc == rank) {
      printf("Rank = %d\n", rank);
      printf("Cart Rank = [%d][%d]", coords[0], coords[1]);
      printf("Local Matrix:\n");
      for (int i = 0; i < sub_rows; i++) {
        for (int j = 0; j < sub_cols; j++) {
          printf("%5.0f ",sub_matrix[i * sub_cols + j]);
          sub_matrix[i * sub_cols + j] ++;
        }
        printf("\n");
      }
      printf("\n");
      printf("Local Vectors:\n");
      for (int i = 0; i < NUM_VEC; i++){
        int vec_len = 0;
        if (i%2 == 0){
          vec_len = sub_cols;
        }
        else{
          vec_len = sub_rows;
        }
        for (int j = 0; j < vec_len; j++){
          printf("%5.0f ", sub_vecs[i][j]);
          sub_vecs[i][j]++;
        }
        printf("\n");
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  /*  start = omp_get_wtime();
  for (int i = 0; i < options.iter; i++){
    for(int j = 0; j < options.n; j++){
      for(int k = 0; k < options.m; k++){
        update(primary, secondary, j, k, vectors);
      }
    }
    swap(&primary, &secondary);
  }
  finish = omp_get_wtime();
  
  double usec_diff = finish - start;
  fprintf(stderr,"loop time = %f\n", usec_diff);
  
  if (options.iter % 2 == 1){
    double **temp = primary;
    primary = secondary;
    secondary = temp;
    for (int i = 0; i < options.n; i++){
      memcpy(primary[i],secondary[i],options.m * sizeof(double));
    } 
  } 
  for (int i = 0; i < options.n; i++){
    free(secondary[i]);
  }
  free(secondary);
  */
}

static void update(double *primary, double *secondary, int j, int k, double **vectors){
  debug("update %d %d\n", j, k);

  double sum = 0;

  if (k-1 < 0) {
    sum += vectors[3][j];
  }
  else{
    sum += primary[(sub_cols * j) + k-1];
  }

  if (j-1 < 0) {
    sum += vectors[0][k];
  }
  else{
    sum += primary[(sub_cols * (j-1)) + k];
  }

  if (k+1 >= options.m) {
    sum += vectors[1][j];
  }
  else{
    sum += primary[(sub_cols * j) + k+1];
  }

  if (j+1 >= options.n) {
    sum += vectors[2][k];
  }
  else{
    sum += primary[(sub_cols * (j+1)) + k];
  }

  secondary[(sub_cols * j) + k] = sum/(double)4;
}

static void swap (double **primary, double **secondary){
  double *temp = *primary;
  *primary = *secondary;
  *secondary = temp;
  /*
  for (int i = 0; i < options.n; i++){
    memcpy((*primary)[i],(*secondary)[i],options.m * sizeof(double));
  }
  */
  
}
