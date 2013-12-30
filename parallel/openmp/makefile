CC = gcc
DEFS = -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS = -std=c99 -pedantic -Wall -fopenmp -Wextra -pedantic-errors -O3 -g -c $(DEFS)
OBJFILES = stencil.o init.o iterate.o
ROW_OBJFILES = stencil.o init.o iterate_row.o
COLUMN_OBJFILES = stencil.o init.o iterate_column.o
DIAGONAL_OBJFILES = stencil.o init.o iterate_diagonal.o
LFLAGS = -fopenmp

all: 	stencil

stencil: 	$(OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

%.o: 	%.c
	$(CC) $(CFLAGS) -o $@ $^

init.c: init.h common.h 

stencil.c: init.h common.h iterate.h

iterate*.c: common.h iterate.h

clean:
	rm -f *.o stencil

row: $(ROW_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^
	
column: $(COLUMN_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

diagonal: $(DIAGONAL_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

