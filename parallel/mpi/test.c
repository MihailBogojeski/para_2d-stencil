#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define COLS  (64)
#define ROWS  (24)

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);
  int p, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  double *a;
  int dims[2] = {0,0};
  MPI_Dims_create(p,2,&dims[0]);

  if (rank == 0){
    a = malloc(sizeof(double) * ROWS * COLS);
  }
  else{
    a = NULL;
  }

  if (COLS > ROWS){
    int temp = dims[0];
    dims[0] = dims[1];
    dims[1] = temp;
  }

  if (p != dims[0]*dims[1]) {
    fprintf(stderr,"Error: number of PEs %d != %d x %d\n", p, dims[0], dims[1]);
    MPI_Finalize();
    exit(-1);
  }
  
  if (ROWS % dims[0] != 0 || COLS % dims[1] != 0){
    fprintf(stderr,"Error: number of p does not divide m or n\n", p, dims[0], dims[1]);
    MPI_Finalize();
    exit(-1);
  }

  int BLOCKROWS = ROWS / dims[0];
  int BLOCKCOLS = COLS / dims[1];

  double *b = malloc(sizeof(double) * BLOCKROWS * BLOCKCOLS);
  for (int ii=0; ii<BLOCKROWS*BLOCKCOLS; ii++) b[ii] = 0;

  if (rank == 0){
    for (int ii=0; ii<ROWS*COLS; ii++) {
      a[ii] = ii;
    }
  } 
  
  MPI_Datatype blocktype;
  MPI_Datatype blocktype2;

  MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_DOUBLE, &blocktype2);
  MPI_Type_create_resized( blocktype2, 0, sizeof(double), &blocktype);
  MPI_Type_commit(&blocktype);

  int disps[dims[0]*dims[1]];
  int counts[dims[0]*dims[1]];
  for (int ii=0; ii<dims[0]; ii++) {
    for (int jj=0; jj<dims[1]; jj++) {
      disps[ii*dims[1]+jj] = ii*COLS*BLOCKROWS+jj*BLOCKCOLS;
      counts [ii*dims[1]+jj] = 1;
    }
  }

  MPI_Scatterv(a, counts, disps, blocktype, b, BLOCKROWS*BLOCKCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  /* each proc prints it's "b" out, in order */
  for (int proc=0; proc<p; proc++) {
    if (proc == rank) {
      printf("Rank = %d\n", rank);
      if (rank == 0) {
        printf("Global matrix: \n");
        for (int ii=0; ii<ROWS; ii++) {
          for (int jj=0; jj<COLS; jj++) {
            printf("%5d ",(int)a[ii*COLS+jj]);
          }
          printf("\n");
        }
      }
      printf("Local Matrix:\n");
      for (int ii=0; ii<BLOCKROWS; ii++) {
        for (int jj=0; jj<BLOCKCOLS; jj++) {
          printf("%5d ",(int)b[ii*BLOCKCOLS+jj]);
        }
        printf("\n");
      }
      printf("\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Finalize();

  return 0;
}
