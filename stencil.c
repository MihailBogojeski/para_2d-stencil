const int n = 100;
const int m = 50;

void update(double **primary, double **secondary, int j, int k, double **vectors);

void init(double **primary, double **vectors);

int main(char **args){
  int iter = 10;
  double **primary;
  double **secondary;
  double **vectors;
  init (primary, vectors);
  for (int i = 0; i < iter; i++){
    for(int j = 0; j < n; j++){
      for(int k = 0; k < m; k++){
        update(primary, secondary, j, k, vectors);
      }
    }
    double **temp = primary;
    primary = secondary;
    secondary = temp;
  }
}

void update(double **primary, double **secondary, int j, int k, double **vectors){
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

void init (double **primary, double **vectors){
  
}
