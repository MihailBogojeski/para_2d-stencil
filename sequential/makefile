CC = gcc
DEFS = -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS = -std=c99 -pedantic -fopenmp -Wall -O3 -g -c $(DEFS)
OBJFILES = stencil.o init.o
LFLAGS = -fopenmp

all: 	stencil

stencil: 	$(OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

%.o: 	%.c
	$(CC) $(CFLAGS) -o $@ $^

init.c: init.h common.h 

stencil.c: init.h common.h 

clean:
	rm -f *.o stencil

