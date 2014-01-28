#include "common.h"
#include "init.h"
#include "iterate.h"


static bool f = false;
static char* prog;




static void free_resources(double **primary, double **vectors);

static void parse_args(int argc, char **argv);

static void print_result(double **primary);

static void print_all(double **primary, double **vectors);

static void usage();

int main(int argc, char **argv){
  prog = argv[0];

  parse_args(argc, argv);

  double **primary = malloc(options.n * sizeof(double*));
  if (primary == NULL){
    bail_out(EXIT_FAILURE, "malloc primary");
  }
  double **vectors = malloc(NUM_VEC * sizeof(double*));
  if (vectors == NULL){
    bail_out(EXIT_FAILURE, "malloc vectors");
  }

  if (f){
    init_file(primary, vectors);
  }
  else{
    init_rand (primary, vectors);
  }

  fprintf(stderr,"init finished\n"); 
  iterate(primary, vectors);
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

static void free_resources(double **primary, double **vectors){
  debug("free_resources\n");
  for (int i = 0; i < options.n; i++){
    free(primary[i]);
  }
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

static void print_all(double **primary, double **vectors){
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

static void print_result(double **primary){
  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
      printf("%3.4f ", primary[i][j]);
    }
    printf("\n");
  }
}

static void usage(){
  bail_out(EXIT_FAILURE, "Usage: stencil rows columns iterations [-f input]");
}
