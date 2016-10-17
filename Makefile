
all: bench distribution

bench: bench.c
	$(CC) -o $@ $< -lpthread

distribution: distribution.c
	$(CC) -o $@ $< 

clean:
	rm -f bench distribution

.PHONY: all clean
