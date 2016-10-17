
all: bench distribution

bench: bench.c
	$(CC) -Irdtsc/ -o $@ $< -lpthread

distribution: distribution.c
	$(CC) -o $@ $< 

clean:
	rm -f bench distribution

.PHONY: all clean
