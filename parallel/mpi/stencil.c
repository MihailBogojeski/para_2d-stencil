#include "common.h"
#include "init.h"
#include "iterate.h"
#include <mpi.h>


static bool f = false;
static char* prog;




static void free_resources(double *primary, double **vectors);

static void parse_args(int argc, char **argv);

static void print_result(double *primary);

static void print_all(double *primary, double **vectors);

static void usage();

int main(int argc, char **argv){
  
  MPI_Init(&argc, &argv);
  int p, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
  prog = argv[0];

  parse_args(argc, argv);

  double *primary = NULL;;
  double **vectors = malloc(NUM_VEC * sizeof(double*));
  if (vectors == NULL){
    bail_out(EXIT_FAILURE, "malloc vectors");
  }

  if (rank == 0){
    primary = malloc(options.m * options.n * sizeof(double));
    
    if (primary == NULL){
      bail_out(EXIT_FAILURE, "malloc primary");
    }
    if (f){
      init_file(primary, vectors);
    }
    else{
      init_rand (primary, vectors);
    }
  }
  else{
    for (int i = 0; i < NUM_VEC; i++){
      vectors[i] = NULL;
    }
  }
  int dims[2] = {0,0};
  MPI_Dims_create(p,2,&dims[0]);
  
  if (options.m > options.n){
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

  int BLOCKROWS = options.n / dims[0];
  int BLOCKCOLS = options.m / dims[1];

  double *b = malloc(sizeof(double) * BLOCKROWS * BLOCKCOLS);
  for (int ii=0; ii<BLOCKROWS*BLOCKCOLS; ii++) b[ii] = 0;

  if (rank == 0){
    for (int i = 0; i < options.n * options.m; i++) {
      primary [i] = i;
    }
  } 
  
  MPI_Datatype blocktype;
  MPI_Datatype blocktype2;

  MPI_Type_vector(BLOCKROWS, BLOCKCOLS, options.m, MPI_DOUBLE, &blocktype2);
  MPI_Type_create_resized( blocktype2, 0, sizeof(double), &blocktype);
  MPI_Type_commit(&blocktype);

  int disps[dims[0]*dims[1]];
  int counts[dims[0]*dims[1]];
  for (int i=0; i < dims[0]; i++) {
    for (int j=0; j<dims[1]; j++) {
      disps[i * dims[1] + j] = (i * options.m * BLOCKROWS) + (j * BLOCKCOLS);
      counts [i * dims[1] + j] = 1;
    }
  }

  MPI_Scatterv(primary, counts, disps, blocktype, b, BLOCKROWS*BLOCKCOLS, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  for (int proc=0; proc<p; proc++) {
    if (proc == rank) {
      printf("Rank = %d\n", rank);
      if (rank == 0) {
        printf("Global matrix: \n");
        for (int i = 0; i < options.n; i++) {
          for (int j = 0; j< options.m; j++) {
            printf("%5.0f ",primary[i * options.m + j]);
          }
          printf("\n");
        }
      }
      printf("Local Matrix:\n");
      for (int i = 0; i < BLOCKROWS; i++) {
        for (int j = 0; j < BLOCKCOLS; j++) {
          printf("%5.0f ",b[i * BLOCKCOLS + j]);
        }
        printf("\n");
      }
      printf("\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  fprintf(stderr,"init finished\n"); 
  //TODO call iteration function
  fprintf(stderr, "loop finished\n");
  
  if (!options.quiet){
    print_result(primary);
  }
  free_resources(primary, vectors);
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

static void free_resources(double *primary, double **vectors){
  debug("free_resources\n");
  free(primary);

  for (int i = 0; i < NUM_VEC; i++){
    free(vectors[i]);
  }
  free(vectors);

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

static void print_all(double *primary, double **vectors){
  for (int i = 0; i < NUM_VEC; i++){
    if (i%2 == 0){
      for (int j = 0; j < options.m; j++){
        printf("%8.4f ", vectors[i][j]);
      }
    }
    else{
      for (int j = 0; j < options.n; j++){
        printf("%8.4f ", vectors[i][j]);
      }
    }
    printf("\n\n");
  }

  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
      printf("%8.4f ", primary[i*options.m + j]);
    }
    printf("\n");
  }
  printf("\n\n");
}

static void print_result(double *primary){
  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
      printf("%3.4f ", primary[i*options.m + j]);
    }
    printf("\n");
  }
}

static void usage(){
  bail_out(EXIT_FAILURE, "Usage: stencil rows columns iterations [-f input]");
}
