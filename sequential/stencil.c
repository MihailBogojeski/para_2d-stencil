#include "common.h"
#include "init.h"
#include <sys/time.h>



static bool f = false;
static char* prog;


void update(double **primary, double **secondary, int j, int k, double **vectors);

void free_resources(double **primary, double **secondary, double **vectors);

void parse_args(int argc, char **argv);

void print_all(double **primary, double **secondary, double **vectors);

void usage();

int main(int argc, char **argv){

  prog = argv[0];
  struct timeval start, finish;
  parse_args(argc, argv);

  double **primary = malloc(options.n * sizeof(double*));
  if (primary == NULL){
    bail_out(EXIT_FAILURE, "malloc primary");
  }
  double **secondary = calloc(options.n, sizeof(double*));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }
  double **vectors = malloc(NUM_VEC * sizeof(double*));
  if (vectors == NULL){
    bail_out(EXIT_FAILURE, "malloc vectors");
  }

  if (f){
    init_file(primary, secondary, vectors);
  }
  else{
    init_rand (primary, secondary, vectors);
  }
  // print_all(primary, secondary, vectors);
  
  fprintf(stderr, "init finished\n");
  gettimeofday(&start,NULL);
  for (int i = 0; i < options.iter; i++){
    for(int j = 0; j < options.n; j++){
      for(int k = 0; k < options.m; k++){
        update(primary, secondary, j, k, vectors);
      }
    }
    double **temp = primary;
    primary = secondary;
    secondary = temp;
    /*
    for (int i = 0; i < options.n; i++){
      for (int j = 0; j < options.m; j++){
        debug("%3.4f ", primary[i][j]);
      }
      debug("\n");
    }
    debug("\n\n");
    */
  }
  gettimeofday(&finish,NULL);
  fprintf(stderr, "loop finished\n");
  long usec_diff = (finish.tv_sec - start.tv_sec)*1000000 + (finish.tv_usec - start.tv_usec);
  fprintf(stderr,"loop time = %lu\n", usec_diff);
/*
  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
      printf("%3.4f ", primary[i][j]);
    }
    printf("\n");
  }
*/
  free_resources(primary, secondary, vectors);
}

void update(double **primary, double **secondary, int j, int k, double **vectors){
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

void parse_args(int argc, char **argv){
  debug("parse args\n");
  char *endptr;

  if (argc < 4 || argc > 6){
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
  while ((c = getopt(argc, argv, "f:")) != -1){
    switch(c){
      case 'f': 
        if (f){
          usage(); 
        }
        f = true;
        options.file = optarg;       
        break;
      case '?':
        usage();
        break;
      default:
        assert(0);
    }
  }
}

void free_resources(double **primary, double **secondary, double **vectors){
  debug("free_resources\n");
  for (int i = 0; i < options.n; i++){
    free(secondary[i]);
    free(primary[i]);
  }
  free(primary);
  free(secondary);

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

void print_all(double **primary, double **secondary, double **vectors){
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
      printf("%8.4f ", primary[i][j]);
    }
    printf("\n");
  }
  printf("\n\n");
}

void usage(){
  bail_out(EXIT_FAILURE, "Usage: stencil rows columns iterations [-f input]");
}
