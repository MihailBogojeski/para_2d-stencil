#include "common.h"
#include "init.h"
#include "iterate.h"


static bool f = false;
static char* prog;




static void free_resources(double *primary, double *sub_matrix);

static void parse_args(int argc, char **argv);

static void print_result(double *primary);

static void print_all(double *primary);

static void usage();

int main(int argc, char **argv){
 

  // Initializing main array and vectors
  MPI_Init(&argc, &argv);
  int p, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
  prog = argv[0];

  parse_args(argc, argv);

  double *primary = NULL;;

  if (rank == 0){
    primary = malloc((options.m + 2) * (options.n + 2) * sizeof(double));
    
    if (primary == NULL){
      bail_out(EXIT_FAILURE, "malloc primary");
    }
    if (f){
      init_file(primary);
    }
    else{
      init_rand (primary);
    }
  }
  
  /*if (rank == 0 && !f){
    for (int i = 0; i < (options.n + 2) * (options.m + 2); i++) {
      primary [i] = i;
    }
  }*/
  MPI_Barrier(MPI_COMM_WORLD);
  

  //Creating new communicators
  MPI_Dims_create(p,2,&dims[0]);
  
  if (options.m >= options.n){
    int temp = dims[0];
    dims[0] = dims[1];
    dims[1] = temp;
  }
  
  if (p != dims[0]*dims[1]) {
    fprintf(stderr,"Error: number of PEs %d != %d x %d\n", p, dims[0], dims[1]);
    MPI_Finalize();
    exit(-1);
  }
  
  if (options.n % dims[0] != 0 || options.m % dims[1] != 0){
    fprintf(stderr,"Error: number of p does not divide m or n\n");
    MPI_Finalize();
    exit(-1);
  }

  int periods[2] = {0,0};


  MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);
  
  MPI_Cart_coords(cart_comm, rank, 2, &coords[0]);


  sub_rows = options.n / dims[0];
  sub_cols = options.m / dims[1];
  
  
  MPI_Barrier(MPI_COMM_WORLD);
 
  double *sub_matrix = calloc((sub_cols+2) * (sub_rows+2), sizeof(double));


  //Create needed MPI types to scatter array and vectors
  
  MPI_Datatype big_submatrix;
  MPI_Datatype big_submatrix2;

  MPI_Type_vector(sub_rows + 2, sub_cols + 2, options.m + 2, MPI_DOUBLE, &big_submatrix2);
  MPI_Type_create_resized(big_submatrix2, 0, sizeof(double), &big_submatrix);
  MPI_Type_commit(&big_submatrix);
  MPI_Type_free(&big_submatrix2);

  int disps[dims[0]*dims[1]];
  int counts[dims[0]*dims[1]];
  for (int i = 0; i < dims[0]; i++) {
    for (int j = 0; j < dims[1]; j++) {
      disps[i * dims[1] + j] = (i * (options.m + 2) * sub_rows) + (j * sub_cols);
      counts [i * dims[1] + j] = 1;
    }
  }
  
  if (rank == 0) fprintf(stderr, "before scatterv\n");
  
  //Scatter array and vectors
  MPI_Scatterv(primary, counts, disps, big_submatrix, 
      sub_matrix, (sub_rows + 2)*(sub_cols +2), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  
  MPI_Type_free(&big_submatrix);
  

  iterate(&sub_matrix);
  
  MPI_Datatype submatrix_recv;
  MPI_Datatype submatrix_recv2;

  MPI_Type_vector(sub_rows, sub_cols, options.m + 2, MPI_DOUBLE, &submatrix_recv2);
  MPI_Type_create_resized(submatrix_recv2, 0, sizeof(double), &submatrix_recv);
  MPI_Type_commit(&submatrix_recv);
  MPI_Type_free(&submatrix_recv2);

  
  MPI_Datatype submatrix_send;
  MPI_Datatype submatrix_send2;

  MPI_Type_vector(sub_rows, sub_cols, sub_cols + 2, MPI_DOUBLE, &submatrix_send2);
  MPI_Type_create_resized(submatrix_send2, 0, sizeof(double), &submatrix_send);
  MPI_Type_commit(&submatrix_send);
  MPI_Type_free(&submatrix_send2);
  
  for (int i = 0; i < dims[0]; i++) {
    for (int j = 0; j < dims[1]; j++) {
      disps[i * dims[1] + j] = (i * (options.m + 2) * sub_rows) + (j * sub_cols) + options.m + 3;
      counts [i * dims[1] + j] = 1;
    }
  }

  //Gather array and vectors 
  MPI_Gatherv(&sub_matrix[sub_cols + 3], 1, submatrix_send, 
      primary, counts, disps, submatrix_recv, 0, MPI_COMM_WORLD);

  if (rank == 0) fprintf(stderr, "after gatherv\n");
  
  MPI_Type_free(&submatrix_send);
  MPI_Type_free(&submatrix_recv);
  MPI_Comm_free(&cart_comm);

  

  //fprintf(stderr,"init finished\n"); 
  //TODO call iteration function
  //fprintf(stderr, "loop finished\n");
  
  if (!options.quiet && rank == 0){
    print_all(primary);
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  free_resources(primary,sub_matrix);
}


static void parse_args(int argc, char **argv){
  debug("parse args\n");
  char *endptr;

  if (argc < 4 || argc > 7){
    usage();
  }

  long rows = strtol(argv[1], &endptr, 0);

  if (endptr == argv[1]){
    bail_out(EXIT_FAILURE, "strtol rows");
  }

  if (rows > INT_MAX){
    bail_out(EXIT_FAILURE, "rows number too big");
  }

  options.n = (int)rows;

  long columns = strtol(argv[2], &endptr, 0);

  if (endptr == argv[2]){
    bail_out(EXIT_FAILURE, "strtol columns");
  }

  if (columns > INT_MAX){
    bail_out(EXIT_FAILURE, "columns number too big");
  }

  options.m = (int)columns;
  
  long iterations = strtol(argv[3], &endptr, 0);

  if (endptr == argv[3]){
    bail_out(EXIT_FAILURE, "strtol iterations");
  }

  if (columns > INT_MAX){
    bail_out(EXIT_FAILURE, "iterations number too big");
  }

  options.iter = (int)iterations;
  char c;
  while ((c = getopt(argc, argv, "qf:")) != -1){
    switch(c){
      case 'f': 
        if (f){
          usage(); 
        }
        f = true;
        options.file = optarg;       
        break;
      case 'q':
        if (options.quiet){
          usage();
        } 
        options.quiet = true;
        break;
      case '?':
        usage();
        break;
      default:
        assert(0);
    }
  }
}

static void free_resources(double *primary, double *sub_matrix){
  debug("free_resources\n");
  free(primary);
  free(sub_matrix);


}

void bail_out(int eval, const char *fmt, ...){
  va_list ap;   
  (void) fprintf(stderr, "%s: ", prog);
  if (fmt != NULL) {
    va_start(ap, fmt);
    (void) vfprintf(stderr, fmt, ap);
    va_end(ap);
  }   
  if (errno != 0) {
    (void) fprintf(stderr, ": %s", strerror(errno));
  }   
  (void) fprintf(stderr, "\n"); 
  exit(eval);
}

static void print_all(double *primary){

  for (int i = 0; i < options.n + 2; i++){
    for (int j = 0; j < options.m + 2; j++){
      printf("%8.4f ", primary[i* (options.m + 2) + j]);
    }
    printf("\n");

  }
  printf("\n\n");
}

static void print_result(double *primary){
  for (int i = 0; i < options.n + 2; i++){
    for (int j = 0; j < options.m + 2; j++){
      printf("%8.4f ", primary[i* (options.m + 2) + j]);
    }
    printf("\n");
  }
}

static void usage(){
  bail_out(EXIT_FAILURE, "Usage: stencil rows columns iterations [-f input]");
}
