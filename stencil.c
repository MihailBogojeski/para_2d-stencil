#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#define RANGE (1000)
#define NUM_VEC (4)
#define DEBUG (0)
#define ITER (50)

#define debug(...) \
  do { if (DEBUG) fprintf(stderr,__VA_ARGS__); } while(0)

static int n;
static int m;
static char* prog;

void update(double **primary, double **secondary, int j, int k, double **vectors);

void init(double **primary, double **secondary, double **vectors);

void free_resources(double **primary, double **secondary, double **vectors);

void parse_args(int argc, char **argv);

void bail_out(int eval, const char *fmt, ...);

double rand_double();

int main(int argc, char **argv){

  prog = argv[0];

  parse_args(argc, argv);

  double **primary = malloc(n * sizeof(double*));
  if (primary == NULL){
    bail_out(EXIT_FAILURE, "malloc primary");
  }
  double **secondary = calloc(n, sizeof(double*));
  if (secondary == NULL){
    bail_out(EXIT_FAILURE, "malloc secondary");
  }
  double **vectors = malloc(NUM_VEC * sizeof(double*));
  if (vectors == NULL){
    bail_out(EXIT_FAILURE, "malloc vectors");
  }

  init (primary, secondary, vectors);

  debug("%f\n", vectors[0][0]);

  for (int i = 0; i < ITER; i++){
    for(int j = 0; j < n; j++){
      for(int k = 0; k < m; k++){
        update(primary, secondary, j, k, vectors);
      }
    }
    double **temp = primary;
    primary = secondary;
    secondary = temp;

    for (int i = 0; i < n; i++){
      for (int j = 0; j < m; j++){
        debug("%3.4f ", primary[i][j]);
      }
      debug("\n");
    }
    debug("\n\n");

  }

  for (int i = 0; i < n; i++){
    for (int j = 0; j < m; j++){
      printf("%3.4f ", primary[i][j]);
    }
    printf("\n");
  }

  free_resources(primary, secondary, vectors);
}

void update(double **primary, double **secondary, int j, int k, double **vectors){
  debug("update %d %d\n", j, k);

  double sum = 0;

  if (k-1 < 0) {
    sum += vectors[0][j];
  }
  else{
    sum += primary[j][k-1];
  }

  if (j-1 < 0) {
    sum += vectors[2][k];
  }
  else{
    sum += primary[j-1][k];
  }

  if (k+1 >= m) {
    sum += vectors[1][j];
  }
  else{
    sum += primary[j][k+1];
  }

  if (j+1 >= n) {
    sum += vectors[3][k];
  }
  else{
    sum += primary[j+1][k];
  }

  secondary[j][k] = sum/(double)4;
}

void parse_args(int argc, char **argv){
  debug("parse args\n");
  char *endptr;

  if (argc != 3){
    bail_out(EXIT_FAILURE, "Usage: stencil rows columns");
  }

  long rows = strtol(argv[1], &endptr, 0);

  if (endptr == argv[1]){
    bail_out(EXIT_FAILURE, "strtol rows");
  }

  if (rows > INT_MAX){
    bail_out(EXIT_FAILURE, "rows number too big");
  }

  n = (int)rows;

  long columns = strtol(argv[2], &endptr, 0);

  if (endptr == argv[2]){
    bail_out(EXIT_FAILURE, "strtol columns");
  }

  if (columns > INT_MAX){
    bail_out(EXIT_FAILURE, "columns number too big");
  }

  m = (int)columns;
}

void init (double **primary, double **secondary, double **vectors){
  debug("init\n");
  srand((unsigned int)time((time_t*) NULL));

  for (int i = 0; i < n; i++){
    primary[i] = malloc(m * sizeof(double));
    if (primary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc primary[%d]", i);
    }
    secondary[i] = calloc(m, sizeof(double));
    if (secondary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc secondary[%d]", i);
    }
    for (int j = 0; j < m; j++){
      primary[i][j] = rand_double();
    }
  }

  int vec_len = 0;
  for (int i = 0; i < NUM_VEC; i++){
    if (i < NUM_VEC/2){
      vec_len = n;
    }
    else{
      vec_len = m;
    }
    vectors[i] = malloc(vec_len * sizeof(double));
    if (vectors[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc vectors[%d]\n", i);
    }
    for (int j = 0; j < vec_len; j++){
      vectors[i][j] = rand_double();
    }
  }
  debug("in init: %f", vectors[0][0]);
}

void free_resources(double **primary, double **secondary, double **vectors){
  debug("free_resources\n");
  for (int i = 0; i < n; i++){
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

double rand_double() 
{
  double range = RANGE; 
  double div = RAND_MAX / range;
  return (rand() / div);
}
