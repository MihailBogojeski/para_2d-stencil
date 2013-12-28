CC = gcc
DEFS = -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS = -std=c99 -pedantic -Wall -g -c $(DEFS)
OBJFILES = stencil.o
LFLAGS =

all: 	stencil

stencil: 	$(OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

%.o: 	%.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o stencil

