all:
	gcc -g -o test_suite -I include src/tests.c src/bigint.c src/rng.c src/chacha.c src/primality.c
clean:
	rm test_suite
