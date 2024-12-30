#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "bigint.h"
#include "rng.h"
#include "chacha.h"
#include "primality.h"

static int successes = 0;
static int failures = 0;

void assert(bool result, char *failure_msg)
{
	if(result){
		successes++;
	} else {
		failures++;
		printf("FAILURE: %s\n", failure_msg);
	}
}

void test_bigint_math_proper()
{
	// start with one word tests
	
	int words = 1;
	struct bigint *a, *b, *res, *expected_res;

	a = bi_init(words);
	b = bi_init(words);
	expected_res = bi_init(words);

	bi_set(a, 5u);
	bi_set(b, 2u);

	assert(!bi_even(a), "bi_even for an odd number failing");
	assert(bi_even(b), "bi_even for an even number failing");

	// addition
	bi_set(expected_res, 7u);
	res = bi_add(a, b);
	assert(bi_eq(res, expected_res), "bigint 1-word addition");
	bi_free(res);

	// subtraction
	bi_set(expected_res, 3u);
	res = bi_sub(a, b);
	assert(bi_eq(res, expected_res), "bigint 1-word subtraction");
	bi_free(res);
	
	// multiplication
	bi_set(expected_res, 10u);
	res = bi_mul(a, b);
	assert(bi_eq(res, expected_res), "bigint 1-word multiplication");
	bi_free(res);

	// integer division
	bi_set(expected_res, 2u);
	res = bi_eucl_div(a, b);
	assert(bi_eq(res, expected_res), "bigint 1-word euclidian division");
	bi_free(res);

	// modulo
	bi_set(expected_res, 1u);
	res = bi_mod(a, b);
	assert(bi_eq(res, expected_res), "bigint 1-word modulo");
	bi_free(res);

	// inc
	bi_set(expected_res, 6u);
	bi_inc(a);
	assert(bi_eq(a, expected_res), "bigint 1-word increment");
	bi_set(a, 5u);

	// mod exp
	// (5 ^ 2) % 4 = 1
	struct bigint *mod = bi_init(words);
	bi_set(mod, 4u);
	bi_set(expected_res, 1u);
	res = bi_mod_exp(a, b, mod);
	assert(bi_eq(res, expected_res), "bigint 1-word modular exponentiation");
	bi_free(res);
	bi_free(mod);
}

void test_bigint(){
	// test create and free
	int test_words = 4;

	struct bigint *x, *y, *z;

	// test init and free
	printf("Testing bi_init\n");
	x = bi_init(test_words);

	bi_printf(x);
	printf("\n");
	printf("Testing bi_free\n");
	bi_free(x);

	// test init_like
	printf("testing bi_init_like\n");
	x = bi_init(test_words);

	y = bi_init_like(x);
	bi_free(x);
	bi_free(y);

	// test set and copy
	x = bi_init(test_words);
	y = bi_init_and_copy(x);

	bi_set(x, 0u);
	bi_set(y, 0u);

	if(!bi_eq(x, y)){
		printf("testing set: x and y are not equal");
		exit(1);
	}

	bi_set(y, 10ul);
	if(bi_eq(x, y)){
		printf("testing set: x and y are equal when they shouldn't be");
		exit(1);
	}

	// test math functions
	// we'll veryify all these things manually just to be sure
	bi_set(x, 2u);

	printf("testing math functions. starting values are x=");
	bi_printf(x);
	printf(", y=");
	bi_printf(y);
	printf("\n");

	z = bi_add(x, y);
	printf("x+y=");
	bi_printf(z);
	printf("\n");
	bi_free(z);


	z = bi_sub(y, x);
	printf("y-x=");
	bi_printf(z);
	printf("\n");
	bi_free(z);

	bi_set(x, 0xFFFFFFFF);
	bi_set(y, 0xFFFFFFFF);
	for(int i = 0; i < 3; i++){
		z = bi_mul(x, y);
		
		printf("mul step %d. x*y=z, where:\nx: ", i);
		bi_printf(x);
		printf("\ny: ");
		bi_printf(y);
		printf("\nz: ");
		bi_printf(z);
		printf("\n");

		bi_copy(z, y);
		bi_free(z);
	}
	
	// only least sig fig example
	bi_set(x, 5u);
	bi_set(y, 3u);
	z = bi_mod(x, y);
	printf("x%%y=");
	bi_printf(z);
	printf("\n");
	bi_free(z);

	// only least sig fig example
	bi_set(x, 5u);
	bi_set(y, 3u);
	z = bi_eucl_div(x, y);
	printf("x/y=");
	bi_printf(z);
	printf("\n");
	bi_free(z);
}

void test_rng(){
	struct will_rng_cfg cfg;
	unsigned int seed = 12345678u;
	int words = 32;
	cfg.words = words;

	struct bigint *res;
	printf("testing init_will_rng\n");
	init_will_rng(&cfg, seed);

	printf("testing will_rng_next\n");
	for(int i = 0; i < 5; i++){
		res = will_rng_next();

		bi_printf(res);
		bi_free(res);
		printf("\n");
	}

	// See how many rng gens we can get done in a second
	unsigned long counter = 0;
	clock_t start = clock();
	while(clock() - start < CLOCKS_PER_SEC){
		res = will_rng_next();
		bi_free(res);
		counter++;
	}

	printf("In one sec, for %d-word numbers, generated %lu nums\n", words, counter);
}

void test_chacha()
{
	printf("Testing chacha, with zero as init\n");

	unsigned int *a, *b;

	a = malloc(16 * sizeof(unsigned int));

	for(int i = 0; i < 5; i++){
		b = malloc(16 * sizeof(unsigned int));

		if(i == 0){
			for(int j = 0; j < 16; j++)
				a[j] = j;
		}

		chacha_block(b, a);

		printf("iter %d\n", i);
		for(int j = 0; j < 16; j++){
			printf("%u", b[j]);
		}
		printf("\n");

		free(a);
		a = b;
	}
}
/*
void test_rsa(){
	int seed = 1234;

	struct rsa_public_token pub;
	struct rsa_private_token priv;

	gen_pub_priv_keys(seed, &pub, &priv);

	printf("seed: %d, pub->e: %d, pub->n: %d, priv->d: %d\n",
	       seed, pub.e, pub.n, priv.d);
}
*/

void test_primality()
{
	struct bigint *test_prime = will_rng_next();

	miller_rabin(test_prime, 1000);
}

void tests(){
	printf("Testing bigint\n");
	test_bigint();

	// printf("Testing rng\n");
	test_rng();

	printf("testing chacha\n");
	test_chacha();

	printf("testing bigint maths\n");
	test_bigint_math_proper();

	printf("Testing primality tests\n");
	test_primality();

	printf("Tests completed!\n");
	printf("Tests: %d. Passes: %d. Failures: %d\n", successes + failures, successes, failures);

	//	test_rsa();
}

int main(){
	tests();
}
