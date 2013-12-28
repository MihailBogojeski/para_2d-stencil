#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#define RANGE (1000)
#define NUM_VEC (4)
#define DEBUG (0)
#define ITER (50)

#define debug(...) \
  do { if (DEBUG) fprintf(stderr,__VA_ARGS__); } while(0)

static bool f = false;
static char* prog;

struct options {
  int n;
  int m;
  char *file;
};

static struct options options;

void update(double **primary, double **secondary, int j, int k, double **vectors);

void init_rand(double **primary, double **secondary, double **vectors);

void init_file(double **primary, double **secondary, double **vectors);

void free_resources(double **primary, double **secondary, double **vectors);

void parse_args(int argc, char **argv);

void bail_out(int eval, const char *fmt, ...);

double rand_double();

void print_all(double **primary, double **secondary, double **vectors);

void usage();

int main(int argc, char **argv){

  prog = argv[0];

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
  print_all(primary, secondary, vectors);

  for (int i = 0; i < ITER; i++){
    for(int j = 0; j < options.n; j++){
      for(int k = 0; k < options.m; k++){
        update(primary, secondary, j, k, vectors);
      }
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

  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
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

  if (argc < 3 || argc > 5){
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

void init_rand(double **primary, double **secondary, double **vectors){
  debug("init\n");
  srand((unsigned int)time((time_t*) NULL));

  for (int i = 0; i < options.n; i++){
    primary[i] = malloc(options.m * sizeof(double));
    if (primary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc primary[%d]", i);
    }
    secondary[i] = calloc(options.m, sizeof(double));
    if (secondary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc secondary[%d]", i);
    }
    for (int j = 0; j < options.m; j++){
      primary[i][j] = rand_double();
    }
  }

  int vec_len = 0;
  for (int i = 0; i < NUM_VEC; i++){
    if (i%2 == 0){
      vec_len = options.m;
    }
    else{
      vec_len = options.n;
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

void init_file(double **primary, double **secondary, double **vectors){

  FILE* input = fopen(options.file, "r");
  if(input == NULL){
    bail_out(EXIT_FAILURE, "File does not exist!");
  }


  for (int i = 0; i < 4; i++){
    int vec_len;
    printf("i : %d \n", i);
    if (i%2 == 0){
      vec_len = options.m;
    }
    else{
      vec_len = options.n;
    }
    
    printf("i : %d \n", i);
    printf("veclen : %d\n", vec_len);
    
    vectors[i] = malloc(vec_len * sizeof(double)); 
    if (vectors[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc vectors[%d]\n", i);
    }
    printf("i : %d \n", i);
    int len = 4096;
    char *line = malloc(len * sizeof(char));
    getline(&line, (size_t *)&len, input);
    if (line == NULL){
      bail_out(EXIT_FAILURE, "getline");
    }
    printf("after getline\n");
    printf("i : %d \n", i);
    char *token = strtok(line, " ");
    if (token == NULL){
      bail_out(EXIT_FAILURE, "strtok");
    }
    printf("i : %d \n", i);
    int j = 0;
    do{
      if (j >= vec_len){
        break;
      }
      char *endptr = NULL;
      double val = strtod(token, &endptr);
      printf("%d : %s\n", j, token);
      if (token == endptr){
        bail_out(EXIT_FAILURE, "file parsing failed!");
      } 
      if (j >= vec_len){
        break;
      }
      vectors[i][j] = val;
      j++;
    printf("i : %d \n", i);
    } while((token = strtok(NULL, " ")) != NULL);
    printf("i : %d \n", i);
    printf("\n\n");
  }

  for (int i = 0; i < options.n; i++){
    primary[i] = malloc(options.m * sizeof(double));
    if (primary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc primary[%d]", i);
    }
    secondary[i] = calloc(options.m, sizeof(double));
    if (secondary[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc secondary[%d]", i);
    }
    
    char *line = NULL;
    int len = 0;
    getline(&line, (size_t *)&len, input);
    if (line == NULL){
      bail_out(EXIT_FAILURE, "getline");
    }
    char *token = strtok(line, " ");
    if (token == NULL){
      bail_out(EXIT_FAILURE, "strtok");
    }
    int j = 0;
    do{
      if (j >= options.m){
        break;
      } 
      char *endptr = NULL;
      double val = strtod(token, &endptr);
      if (token == endptr){
        bail_out(EXIT_FAILURE, "file parsing failed!");
      } 

      primary[i][j] = val;
      j++;
    } while((token = strtok(NULL, " ")) != NULL);
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

double rand_double() 
{
  double range = RANGE; 
  double div = RAND_MAX / range;
  return (rand() / div);
}

void print_all(double **primary, double **secondary, double **vectors){
  for (int i = 0; i < NUM_VEC; i++){
    if (i%2 == 0){
      for (int j = 0; j < options.m; j++){
        printf("%04.4f ", vectors[i][j]);
      }
    }
    else{
      for (int j = 0; j < options.n; j++){
        printf("%04.4f ", vectors[i][j]);
      }
    }
    printf("\n\n");
  }

  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
      printf("%04.4f ", primary[i][j]);
    }
    printf("\n");
  }
  printf("\n\n");
}

void usage(){
  bail_out(EXIT_FAILURE, "Usage: stencil rows columns [-f input]");
}
