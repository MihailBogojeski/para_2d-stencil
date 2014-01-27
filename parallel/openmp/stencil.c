#include "common.h"
#include "init.h"
#include "iterate.h"


static bool f = false;
static char* prog;




static void free_resources(double **primary);

static void parse_args(int argc, char **argv);

static void print_all(double **primary);

static void usage();

int main(int argc, char **argv){
  prog = argv[0];

  parse_args(argc, argv);

  double **primary = malloc((options.n + 2) * sizeof(double*));
  if (primary == NULL){
    bail_out(EXIT_FAILURE, "malloc primary");
  }

  if (f){
    init_file(primary);
  }
  else{
    init_rand (primary);
  }

  //print_all(primary);

  fprintf(stderr,"init finished\n"); 
  iterate(primary);
  fprintf(stderr, "loop finished\n");
  
  if (!options.quiet){
    print_all(primary);
  }
  free_resources(primary);
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

static void free_resources(double **primary){
  debug("free_resources\n");
  for (int i = 0; i < options.n + 2; i++){
    free(primary[i]);
  }
  free(primary);


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

static void print_all(double **primary){
  for (int i = 0; i < options.n + 2; i++){
    for (int j = 0; j < options.m + 2; j++){
      printf("%8.4f ", primary[i][j]);
    }
    printf("\n");
  }
  printf("\n\n");
}


static void usage(){
  bail_out(EXIT_FAILURE, "Usage: stencil rows columns iterations [-f input]");
}
