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
#include <mpi.h>

#define NUM_VEC (4)
#define DEBUG (0)
#define ROW_VEC (options.n + 2)
#define COL_VEC (options.m + 2)
#define SUB_ROW (sub_rows + 2)
#define SUB_COL (sub_cols + 2)

#define debug(...) \
  do { if (DEBUG) fprintf(stderr,__VA_ARGS__); } while(0)

struct options {
  int n;
  int m;
  int iter;
  char *file;
  bool quiet;
};

struct options options;

int sub_rows, sub_cols, dims[2], coords[2];

MPI_Comm cart_comm;

void bail_out(int eval, const char *fmt, ...);
