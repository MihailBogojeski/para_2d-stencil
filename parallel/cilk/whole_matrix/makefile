CC = cilkc 
DEFS =  -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=200809L
CFLAGS = -pedantic -Wall -O2 -c $(DEFS)
OBJFILES = stencil.o init.o iterate.o
ROW_OBJFILES = stencil.o init.o iterate_row.o

all: 	row

%.o: 	%.cilk
	$(CC) $(CFLAGS) -o $@ $^

init.cilk: init.h common.h 

stencil.cilk: init.h common.h iterate.cilkh

iterate*.cilk: common.h iterate.cilkh

clean:
	rm -f *.o stencil row space

row: $(ROW_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^
	
