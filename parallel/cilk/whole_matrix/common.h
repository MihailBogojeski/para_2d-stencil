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

#define NUM_VEC (4)
#define DEBUG (0)
#define ROW_VEC (options.n + 2)
#define COL_VEC (options.m + 2)

#define debug(...) \
  do { if (DEBUG) fprintf(stderr,__VA_ARGS__); } while(0)

struct options {
  int n;
  int m;
  int iter;
  char *file;
  int quiet;
};

struct options options;


void bail_out(int eval, const char *fmt, ...);
