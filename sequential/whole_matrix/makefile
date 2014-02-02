CC = gcc
DEFS = -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS = -std=c99 -pedantic -Wall -fopenmp -O3 -g -c $(DEFS)
OBJFILES = stencil.o init.o iterate.o
RAND_OBJFILES = generate_rand.o init.o
ROW_OBJFILES = stencil.o init.o iterate_row.o
COLUMN_OBJFILES = stencil.o init.o iterate_column.o
DIAGONAL_OBJFILES = stencil.o init.o iterate_diagonal.o
SPACE_OBJFILES = stencil.o init.o iterate_row_space.o
LFLAGS = -fopenmp

all: row column space diagonal

stencil: 	$(OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

%.o: 	%.c
	$(CC) $(CFLAGS) -o $@ $^

init.c: init.h common.h 

stencil.c: init.h common.h iterate.h

generate_rand.c: init.h common.h

iterate*.c: common.h iterate.h

clean:
	rm -f *.o stencil space row column diagonal rand_gen

row: $(ROW_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^
	
column: $(COLUMN_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

diagonal: $(DIAGONAL_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

space: $(SPACE_OBJFILES)
	$(CC) $(LFLAGS) -o $@ $^

rand_gen: $(RAND_OBJFILES)
	$(CC) $(LFLAGS) -o $@ $^
