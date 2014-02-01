CC = mpicc
DEFS = -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS = -std=c99 -pedantic -Wall -O3 -g -c $(DEFS)
OBJFILES = stencil.o init.o 
SR_OBJFILES = stencil.o init.o iterate_sendrecv.o
NON_BLK_OBJFILES = stencil.o init.o iterate_non_blocking.o
ONE_SIDE_OBJFILES = stencil.o init.o iterate_one_sided.o
ONE_SIDE_GET_OBJFILES = stencil.o init.o iterate_one_sided_get.o
LFLAGS = 
all: sendrecv non_blocking one_sided one_sided_get	

stencil: 	$(OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

%.o: 	%.c
	$(CC) $(CFLAGS) -o $@ $^

init.c: init.h common.h 

stencil.c: init.h common.h 

iterate*.c: common.h iterate.h

clean:
	rm -f *.o sendrecv non_blocking one_sided* 

sendrecv: $(SR_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^
	
non_blocking: $(NON_BLK_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

one_sided: $(ONE_SIDE_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

one_sided_get: $(ONE_SIDE_GET_OBJFILES) 
	$(CC) $(LFLAGS) -o $@ $^

