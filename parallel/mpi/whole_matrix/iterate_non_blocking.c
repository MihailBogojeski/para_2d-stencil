#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double *primary, double *secondary, int j, int k);
static void swap (double **primary, double **secondary);

void iterate(double **sub_matrix)
{
  double start, finish;
  int p, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Request requests[8];
  MPI_Status statuses[8];

  double *secondary = calloc((sub_rows + 2) * (sub_cols + 2), sizeof(double));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }

  memcpy(secondary, *sub_matrix,(sub_rows + 2) * (sub_cols + 2) * sizeof(double));
  
  /* for (int proc = 0; proc < p; proc++){
    if (proc == rank){
      printf("submatrix:\n");
      printf("Rank : %d\n", rank);
      printf("matrix : \n");
      for(int j = 0; j < sub_rows +2; j++){
        for(int k = 0; k < sub_cols+ 2; k++){
          printf("%8.4f ", sub_matrix[j *  (sub_cols +2) + k]);
        }
        printf("\n");
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  for (int proc = 0; proc < p; proc++){
    if (proc == rank){
      printf("secondary:\n");
      printf("Rank : %d\n", rank);
      printf("matrix : \n");
      for(int j = 0; j < sub_rows +2; j++){
        for(int k = 0; k < sub_cols+ 2; k++){
          printf("%8.4f ", secondary[j *  (sub_cols +2) + k]);
        }
        printf("\n");
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }*/

  MPI_Datatype col, col2; 
  MPI_Type_vector(sub_rows + 2,1,sub_cols + 2,MPI_DOUBLE, &col2); 
  MPI_Type_create_resized(col2, 0, sizeof(double), &col);
  MPI_Type_commit(&col); 

  if (rank == 0) fprintf(stderr, "init finished\n");
  start = MPI_Wtime();
  for (int i = 0 ; i < options.iter; i++){

    for (int j = 1; j <= sub_rows; j++){
      for(int k = 1; k <= sub_cols; k++){
        update(*sub_matrix, secondary, j,k);
      }
    }
    swap(sub_matrix, &secondary);
  
    int reqs = 0;    
    if (coords[0] > 0){
      int tmp_coords[2] = {coords[0] - 1, coords[1]};
      int tmp_rank = 0;
      MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
      MPI_Isend (&((*sub_matrix)[sub_cols + 2]), sub_cols + 2, MPI_DOUBLE, tmp_rank, 2, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
      MPI_Irecv (*sub_matrix, sub_cols + 2, MPI_DOUBLE, tmp_rank, 0, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
    }

    if (coords[0] < dims[0] - 1){
      int tmp_coords[2] = {coords[0] + 1, coords[1]};
      int tmp_rank = 0;
      MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
      MPI_Isend (&((*sub_matrix)[(sub_cols + 2) * (sub_rows)]), sub_cols + 2, MPI_DOUBLE, tmp_rank, 0, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
      MPI_Irecv (&((*sub_matrix)[(sub_cols + 2) * (sub_rows + 1)]), sub_cols + 2, MPI_DOUBLE, tmp_rank, 2, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
    }

    if (coords[1] > 0){
      int tmp_coords[2] = {coords[0], coords[1] - 1};
      int tmp_rank = 0;
      MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
      MPI_Isend (&((*sub_matrix)[1]), 1, col, tmp_rank, 1, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
      MPI_Irecv (*sub_matrix, 1, col, tmp_rank, 3, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
    }

    if (coords[1] < dims[1] - 1){
      int tmp_coords[2] = {coords[0], coords[1] + 1};
      int tmp_rank = 0;
      MPI_Cart_rank(cart_comm, &tmp_coords[0], &tmp_rank);
      MPI_Isend (&((*sub_matrix)[sub_cols]), 1, col, tmp_rank, 3, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
      MPI_Irecv (&((*sub_matrix)[sub_cols + 1]), 1, col, tmp_rank, 1, MPI_COMM_WORLD, &requests[reqs]);
      reqs++;
    }

    MPI_Waitall(reqs,requests,statuses);
  }
  finish = MPI_Wtime();
  if (rank == 0) fprintf(stderr, "loop time = %f\n", finish-start);
  
  if (rank == 0) fprintf(stderr, "loop finished\n");

  
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
  }*/
  free(secondary);
}

static void update(double *primary, double *secondary, int j, int k){
  debug("update %d %d\n", j, k);

  double sum = 0;

  sum += primary[((sub_cols + 2) * j) + k-1];
  sum += primary[((sub_cols + 2) * (j-1)) + k];
  sum += primary[((sub_cols + 2) * j) + k+1];
  sum += primary[((sub_cols + 2) * (j+1)) + k];

  secondary[((sub_cols + 2) * j) + k] = sum/(double)4;
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
