#include "common.h"
#include <omp.h>
#include <sys/time.h>

static void update(double *primary, double *secondary, int j, int k);
static void swap (double **primary, double **secondary);
static void update_mem(double *mpi_mem, double *sub_matrix);

void iterate(double **sub_matrix)
{
  double start, finish;
  int p, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  double *mpi_mem;
  
  //initialize window and mpi memory segment
  MPI_Win win;
  MPI_Alloc_mem(2 * (SUB_ROW + SUB_COL) * sizeof(double) , MPI_INFO_NULL, &mpi_mem);


  MPI_Win_create(mpi_mem,2 * (SUB_ROW + SUB_COL) * sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
  MPI_Group neighbours, world;
  MPI_Comm_group(MPI_COMM_WORLD, &world);
  int neigh_ranks[4];
  int tmp_coords[2];
  int num_neigh = 0;
  
  //top
  if (coords[0] > 0){
    tmp_coords[0] = coords[0] - 1;
    tmp_coords[1] = coords[1];
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &neigh_ranks[num_neigh]);
    num_neigh++;
  }
  //bottom
  if (coords[0] < dims[0] - 1){
    tmp_coords[0] = coords[0] + 1;
    tmp_coords[1] = coords[1];
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &neigh_ranks[num_neigh]);
    num_neigh++;
  }
  //left
  if (coords[1] > 0){
    tmp_coords[0] = coords[0];
    tmp_coords[1] = coords[1] - 1;
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &neigh_ranks[num_neigh]);
    num_neigh++;
  }
  //right
  if (coords[1] < dims[1] - 1){
    tmp_coords[0] = coords[0];
    tmp_coords[1] = coords[1] + 1;
    MPI_Cart_rank(cart_comm, &tmp_coords[0], &neigh_ranks[num_neigh]);
    num_neigh++;
  }


  MPI_Group_incl(world, num_neigh, neigh_ranks, &neighbours);

  double *secondary = calloc((SUB_ROW) * (SUB_COL), sizeof(double));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }

  memcpy(secondary, *sub_matrix,(SUB_ROW) * (SUB_COL) * sizeof(double));

  //Create column datatype
  MPI_Datatype col, col2; 
  MPI_Type_vector(SUB_ROW,1,SUB_COL,MPI_DOUBLE, &col2); 
  MPI_Type_create_resized(col2, 0, sizeof(double), &col);
  MPI_Type_commit(&col); 

  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0) fprintf(stderr, "init finished\n");
  start = MPI_Wtime();

  //start iterations
  for (int i = 0 ; i < options.iter; i++){

    for (int j = 1; j <= sub_rows; j++){
      for(int k = 1; k <= sub_cols; k++){
        update(*sub_matrix, secondary, j,k);
      }
    }
    swap(sub_matrix, &secondary);
  
    update_mem(mpi_mem, *sub_matrix);
 
    //MPI_Win_start(neighbours, 0, win);
    //MPI_Win_post(neighbours, 0, win);
    MPI_Win_fence(0,win);
    int get_num = 0;

    //exchange up
    if (coords[0] > 0){
      MPI_Get(*sub_matrix, SUB_COL, MPI_DOUBLE, neigh_ranks[get_num], SUB_COL, SUB_COL, MPI_DOUBLE, win);     
      get_num++;
    }

    //exchange down
    if (coords[0] < dims[0] - 1){
      MPI_Get(&((*sub_matrix)[(SUB_COL) * (sub_rows + 1)]), SUB_COL, MPI_DOUBLE, neigh_ranks[get_num] , 0, SUB_COL, MPI_DOUBLE, win);
      get_num++;
    }

    //exchange left
    if (coords[1] > 0){
      MPI_Get(*sub_matrix, 1, col, neigh_ranks[get_num] , 2*SUB_COL + SUB_ROW, SUB_ROW, MPI_DOUBLE, win);
      get_num++;
    }

    //exchange right
    if (coords[1] < dims[1] - 1){
      MPI_Get(&((*sub_matrix)[sub_cols + 1]), 1, col, neigh_ranks[get_num], 2*SUB_COL, SUB_ROW, MPI_DOUBLE, win);     
      get_num++;
    }

    //MPI_Win_wait(win);
    //MPI_Win_complete(win);
    MPI_Win_fence(0,win);
    /*for (int proc = 0; proc < p; proc++){
      if (proc == rank){
        printf("\n\n");
        printf("Rank : %d\n", rank);
        printf("matrix : \n");
        for(int j = 0; j < SUB_ROW; j++){
          for(int k = 0; k < SUB_COL; k++){
            printf("%8d ", (int)(*sub_matrix)[j *  (SUB_COL) + k]);
          }
          printf("\n");
        }
        printf("memory:\n");
        for(int j = 0; j < 2; j++){
          for(int k = 0; k< SUB_COL; k++){
            printf("%8d ", (int)mpi_mem[j *  (SUB_COL) + k]);
          }
          printf("\n");
        }
        for(int j = 0; j < 2; j++){
          for(int k = 0; k< SUB_ROW; k++){
            printf("%8d ", (int)mpi_mem[2*SUB_COL + j*(SUB_ROW) + k]);
          }
          printf("\n");
        }
        printf("\n");
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }*/
  }
  MPI_Barrier(MPI_COMM_WORLD);
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
  MPI_Win_free(&win);
  MPI_Free_mem(mpi_mem);
}

static void update(double *primary, double *secondary, int j, int k){
  debug("update %d %d\n", j, k);

  double sum = 0;

  sum += primary[((SUB_COL) * j) + k-1];
  sum += primary[((SUB_COL) * (j-1)) + k];
  sum += primary[((SUB_COL) * j) + k+1];
  sum += primary[((SUB_COL) * (j+1)) + k];

  secondary[((SUB_COL) * j) + k] = sum/(double)4;
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

static void update_mem(double *mpi_mem, double *sub_matrix){
  
  memcpy(mpi_mem, &(sub_matrix[SUB_COL]), SUB_COL * sizeof(double));
  memcpy(&mpi_mem[SUB_COL],&(sub_matrix[(SUB_COL) * (sub_rows)]), SUB_COL * sizeof(double));
  for (int i = 0; i < SUB_ROW; i++){
    mpi_mem[2*SUB_COL + i] = sub_matrix[(SUB_COL * i) + 1];
  }
  for (int i = 0; i < SUB_ROW; i++){
    mpi_mem[2*SUB_COL + SUB_ROW + i] =sub_matrix[(SUB_COL * i) + sub_cols];
  }

}
