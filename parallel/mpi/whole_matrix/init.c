#include "common.h"
#include "init.h" 

#define RANGE (1000)

double rand_double();


void init_rand(double *primary){
  debug("init\n");
  srand((unsigned int)time((time_t*) NULL));

  for (int i = 0; i < ROW_VEC; i++){
    for (int j = 0; j < COL_VEC; j++){
      primary[i*(COL_VEC) + j] = rand_double();
    }
  }

}

void init_file(double *primary){
  FILE* input = fopen(options.file, "r");
  if(input == NULL){
    bail_out(EXIT_FAILURE, "File does not exist!");
  }
  
  for (int i = 0; i < ROW_VEC; i++){
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
      char *endptr = NULL;
      double val = strtod(token, &endptr);
      if (token == endptr){
        bail_out(EXIT_FAILURE, "file parsing failed!");
      } 

      primary[i*(COL_VEC) + j] = val;
      j++;
      if (j >= COL_VEC){
        break;
      } 
    } while((token = strtok(NULL, " ")) != NULL);
  }
}

double rand_double() 
{
  double range = RANGE; 
  double div = RAND_MAX / range;
  return (rand() / div);
}


