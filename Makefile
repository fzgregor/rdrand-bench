
all: bench distribution

bench: bench.c
	$(CC) $(CFLAGS) -Irdtsc/ -o $@ $< -lpthread

distribution: distribution.c
	$(CC) $(CFLAGS) -o $@ $< 

clean:
	rm -f bench distribution

.PHONY: all clean
