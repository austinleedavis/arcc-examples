CC=icc
CFLAGS=-std=c99 -O3 -mmic -vec-report3 -openmp
EXE=matrix.mic

$(EXE) : *.c
	$(CC) -o $@ $< $(CFLAGS) 

.PHONY: clean

clean:
	rm $(EXE)
