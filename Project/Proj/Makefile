CC = mpicc -lm
CFLAGS = -Wall -O3 -g
BINS = shearsort

all: $(BINS)

shearsort: shearsort.c
	$(CC) $(CFLAGS) -o $@ shearsort.c 

clean:
	$(RM) $(BINS) out.txt