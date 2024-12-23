all:
	gcc -g -o test_suite tests.c bigint.c rng.c


clean:
	rm test_suite
