#include "common.h"
#include "init.h" 

#define RANGE (1000)

double rand_double();


void init_rand(double *primary, double **vectors){
  debug("init\n");
  srand((unsigned int)time((time_t*) NULL));

  for (int i = 0; i < options.n; i++){
    for (int j = 0; j < options.m; j++){
      primary[i*options.m + j] = rand_double();
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

void init_file(double *primary, double **vectors){
  FILE* input = fopen(options.file, "r");
  if(input == NULL){
    bail_out(EXIT_FAILURE, "File does not exist!");
  }


  for (int i = 0; i < 4; i++){
    int vec_len;
    debug("i : %d \n", i);
    if (i%2 == 0){
      vec_len = options.m;
    }
    else{
      vec_len = options.n;
    }
    
    debug("i : %d \n", i);
    debug("veclen : %d\n", vec_len);
    
    vectors[i] = malloc(vec_len * sizeof(double)); 
    if (vectors[i] == NULL){
      bail_out(EXIT_FAILURE, "malloc vectors[%d]\n", i);
    }
    debug("i : %d \n", i);
    int len = 4096;
    char *line = malloc(len * sizeof(char));
    getline(&line, (size_t *)&len, input);
    if (line == NULL){
      bail_out(EXIT_FAILURE, "getline");
    }
    debug("after getline\n");
    debug("i : %d \n", i);
    char *token = strtok(line, " ");
    if (token == NULL){
      bail_out(EXIT_FAILURE, "strtok");
    }
    debug("i : %d \n", i);
    int j = 0;
    do{
      if (j >= vec_len){
        break;
      }
      char *endptr = NULL;
      double val = strtod(token, &endptr);
      debug("%d : %s\n", j, token);
      if (token == endptr){
        bail_out(EXIT_FAILURE, "file parsing failed!");
      } 
      if (j >= vec_len){
        break;
      }
      vectors[i][j] = val;
      j++;
    debug("i : %d \n", i);
    } while((token = strtok(NULL, " ")) != NULL);
    debug("i : %d \n", i);
    debug("\n\n");
  }
  

  for (int i = 0; i < options.n; i++){
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

      primary[i*options.m + j] = val;
      j++;
    } while((token = strtok(NULL, " ")) != NULL);
  }
}

double rand_double() 
{
  double range = RANGE; 
  double div = RAND_MAX / range;
  return (rand() / div);
}


