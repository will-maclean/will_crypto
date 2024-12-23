all:
	gcc -g -o src/test_suite -I include src/tests.c src/bigint.c src/rng.c


clean:
	rm test_suite
