#include "common.h"
#include "init.h"
#include "iterate.h"


static bool f = false;
static char* prog;




static void free_resources(double *primary, double **vectors);

static void parse_args(int argc, char **argv);

static void print_result(double *primary);

static void print_all(double *primary, double **vectors);

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
  
  if (rank == 0 && !f){
    for (int i = 0; i < options.n * options.m; i++) {
      primary [i] = i;
    }
    int k = 0;
    for (int i = 0; i < NUM_VEC; i++){
      int vec_len = 0;
      if (i%2 == 0){
        vec_len = options.m;
      }
      else{
        vec_len = options.n;
      }
      for (int j = 0; j < vec_len; j++){
        vectors[i][j] = k;
        k++;
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  
  if (rank == 0){
    MPI_Send(vectors[1], options.n, MPI_DOUBLE, p-1, 10, MPI_COMM_WORLD);
    MPI_Send(vectors[2], options.m, MPI_DOUBLE, p-1, 11, MPI_COMM_WORLD);
  }
  else if (rank == p-1){
    vectors[1] = calloc(options.n, sizeof(double)); 
    if (vectors[1] == NULL){
      bail_out(EXIT_FAILURE, "malloc vectors[1]");
    }
    vectors[2] = calloc(options.m, sizeof(double)); 
    if (vectors[2] == NULL){
      bail_out(EXIT_FAILURE, "malloc vectors[2]");
    }
    MPI_Status status;
    MPI_Recv(vectors[1],options.n, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD, &status);
    MPI_Recv(vectors[2],options.m, MPI_DOUBLE, 0, 11, MPI_COMM_WORLD, &status);
  }

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

  MPI_Comm vec_comms[NUM_VEC];
  MPI_Group orig_gr, vec_groups[NUM_VEC];
  int **vec_ranks = malloc(NUM_VEC * sizeof(int*));
  if (vec_ranks == NULL){
      bail_out(EXIT_FAILURE, "malloc vec_ranks");
    }
  for (int i = 0; i < NUM_VEC; i++){
    int vec_len = 0;
    if (i%2 == 0){
      vec_len = dims[1];
    }
    else{
      vec_len = dims[0];
    }
    vec_ranks[i] = calloc(vec_len, sizeof(int));  
    if (vec_ranks[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc vec_ranks[%d]", i);
    }
  }
  


  MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);
  
  MPI_Cart_coords(cart_comm, rank, 2, &coords[0]);


  sub_rows = options.n / dims[0];
  sub_cols = options.m / dims[1];
  
  int tmp_coords[2];
  int tmp_rank;


  //Initializing rank arrays
  for (int i = 0; i < dims[0]; i++){
    for (int j = 0; j < dims[1]; j++){
      tmp_coords[0] = i;
      tmp_coords[1] = j;
      
      MPI_Cart_rank(cart_comm, &tmp_coords[0],&tmp_rank);
    
      if (i == 0){
        vec_ranks[0][j] = tmp_rank; 
      }
      if (i == dims[0] - 1){
        vec_ranks[2][j] = tmp_rank; 
      }
      if (j == 0){
        vec_ranks[3][i] = tmp_rank; 
      }
      if (j == dims[1] - 1){
        vec_ranks[1][i] = tmp_rank; 
      }
    }
  }
  
  if (rank == 0){
    printf("top : ");
    for (int i = 0; i < dims[1]; i++){
      printf("%3d", vec_ranks[0][i]);
    }
    printf("\n\n");
    printf("bottom : ");
    for (int i = 0; i < dims[1]; i++){
      printf("%3d", vec_ranks[2][i]);
    }
    printf("\n\n");
    printf("left : ");
    for (int i = 0; i < dims[0]; i++){
      printf("%3d", vec_ranks[3][i]);
    }
    printf("\n\n");
    printf("right : ");
    for (int i = 0; i < dims[0]; i++){
      printf("%3d", vec_ranks[1][i]);
    }
    printf("\n\n");
  }
  
  
  //Initializing vector groups
  MPI_Comm_group(MPI_COMM_WORLD, &orig_gr); 
   
  for (int i = 0; i < NUM_VEC; i++){
    int dims_len = 0;
    if(i%2 == 0){
      dims_len = dims[1];
    }
    else{
      dims_len = dims[0];
    }
    MPI_Group_incl(orig_gr, dims_len, vec_ranks[i], &vec_groups[i]);
  }
  
  for (int i = 0; i < NUM_VEC; i++){
    MPI_Comm_create(MPI_COMM_WORLD, vec_groups[i], &vec_comms[i]);
    if (vec_comms[i] == MPI_COMM_NULL){
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
 

  //Initializing local matrices and vectors
  double *sub_matrix = calloc(sub_rows * sub_cols, sizeof(double));
    if (sub_matrix == NULL){
      bail_out(EXIT_FAILURE, "malloc sub_matrix");
    }

  double **sub_vecs = malloc(sizeof(double*) * NUM_VEC);
    if (sub_vecs == NULL){
      bail_out(EXIT_FAILURE, "malloc sub_vecs");
    }
  for (int i = 0; i < NUM_VEC; i++){
    int vec_len = 0;
    if (i%2 == 0){
      vec_len = sub_cols;
    }
    else{
      vec_len = sub_rows;
    }
    sub_vecs[i] = calloc(vec_len, sizeof(double));  
    if (sub_vecs[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc sub_vecs[%d]", i);
    }
  }


  //Create needed MPI types to scatter array and vectors
  
  MPI_Datatype blocktype;
  MPI_Datatype blocktype2;

  MPI_Type_vector(sub_rows, sub_cols, options.m, MPI_DOUBLE, &blocktype2);
  MPI_Type_create_resized(blocktype2, 0, sizeof(double), &blocktype);
  MPI_Type_commit(&blocktype);
  MPI_Type_free(&blocktype2);

  int disps[dims[0]*dims[1]];
  int counts[dims[0]*dims[1]];
  for (int i = 0; i < dims[0]; i++) {
    for (int j = 0; j < dims[1]; j++) {
      disps[i * dims[1] + j] = (i * options.m * sub_rows) + (j * sub_cols);
      counts [i * dims[1] + j] = 1;
    }
  }
  
  //Scatter array and vectors
  MPI_Scatterv(primary, counts, disps, blocktype, 
      sub_matrix, sub_rows*sub_cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  

  
  for (int i = 0; i < NUM_VEC; i++){
    int svec_len = 0;
    int root = 0;
    if (i%2 == 0){
      svec_len = sub_cols;
      if (i == 2){
        root = dims[1]-1; 
      }
    }
    else{
      svec_len = sub_rows;
      if (i == 1){
        root = dims[0]-1; 
      }
    }
    if (vec_comms[i] != MPI_COMM_NULL){
      MPI_Scatter(vectors[i], svec_len, MPI_DOUBLE, sub_vecs[i], svec_len, MPI_DOUBLE, root, vec_comms[i]);
      //printf("still ok %d, rank = %d\n", i, rank);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("survived scatter %d\n", i, rank);
  }

  iterate(sub_matrix, sub_vecs);

  
  //Gather array and vectors 
  MPI_Gatherv(sub_matrix, sub_rows*sub_cols, MPI_DOUBLE, 
      primary, counts, disps, blocktype, 0, MPI_COMM_WORLD);

  for (int i = 0; i < NUM_VEC; i++){
    int svec_len = 0;
    int root = 0;
    if (i%2 == 0){
      svec_len = sub_cols;
      if (i == 2){
        root = dims[1]-1; 
      }
    }
    else{
      svec_len = sub_rows;
      if (i == 1){
        root = dims[0]-1; 
      }
    }
    if (vec_comms[i] != MPI_COMM_NULL){
      MPI_Gather(sub_vecs[i], svec_len, MPI_DOUBLE, vectors[i], svec_len, MPI_DOUBLE, root, vec_comms[i]);
      //printf("still ok %d, rank = %d\n", i, rank);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //printf("survived gather %d\n", i, rank);
  }
  
  if (rank == 0){
    MPI_Status status;
    MPI_Recv(vectors[1],options.n, MPI_DOUBLE, p-1, 10, MPI_COMM_WORLD, &status);
    MPI_Recv(vectors[2],options.m, MPI_DOUBLE, p-1, 11, MPI_COMM_WORLD, &status);
  }
  else if (rank == p-1){
    MPI_Send(vectors[1], options.n, MPI_DOUBLE, 0, 10, MPI_COMM_WORLD);
    MPI_Send(vectors[2], options.m, MPI_DOUBLE, 0, 11, MPI_COMM_WORLD);
  }
  
      
  if (rank == 0) {
    printf("Global matrix: \n");
    for (int i = 0; i < options.n; i++) {
      for (int j = 0; j< options.m; j++) {
        printf("%5.0f ",primary[i * options.m + j]);
      }
      printf("\n");
    }
    printf("\n");
    printf("Global vectors: \n");
    for (int i = 0; i < NUM_VEC; i++){
      int vec_len = 0;
      if (i%2 == 0){
        vec_len = options.m;
      }
      else{
        vec_len = options.n;
      }
      for (int j = 0; j < vec_len; j++){
        printf("%5.0f ", vectors[i][j]);
      }
      printf("\n");
    }
  }
  
  MPI_Barrier(MPI_COMM_WORLD);

  //fprintf(stderr,"init finished\n"); 
  //TODO call iteration function
  //fprintf(stderr, "loop finished\n");
  
  if (!options.quiet && rank == 0){
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
