#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"
#include "rng.h"


void test_bigint(){
	// test create and free
	int test_words = 4;

	struct bigint *x, *y, *z;

	// test init and free
	printf("Testing bi_init\n");
	enum bi_op_result init_res = bi_init(&x, test_words);

	if(init_res != OKAY){
		printf("Failed to initialise bigint. Got error: %d\n", init_res);
		exit(1);
	}
	bi_printf(x);
	printf("\n");
	printf("Testing bi_free\n");
	bi_free(x);

	// test init_like
	printf("testing bi_init_like\n");
	bi_init(&x, test_words);

	init_res = bi_init_like(&y, x);

	if(init_res != OKAY){
		printf("Failed to init_like bigint. Got error: %d", init_res);
		exit(1);
	}

	bi_free(x);
	bi_free(y);

	// test set and copy
	bi_init(&x, test_words);
	bi_init_like(&y, x);

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

	bi_add(x, y, &z);
	printf("x+y=");
	bi_printf(z);
	printf("\n");
	bi_free(z);


	bi_sub(y, x, &z);
	printf("y-x=");
	bi_printf(z);
	printf("\n");
	bi_free(z);


	bi_mul(x, y, &z);
	printf("x*y=");
	bi_printf(z);
	printf("\n");
	bi_free(z);
}

void test_rng(){
	struct will_rng_cfg cfg;
	struct will_rng_state state;

	unsigned int seed = 12345678u;

	/* poses an interesting question about the best way to choose a, b, and
	 * m to be cryptographically secure. Are there default values that
	 * always work best? Or should the be randomly set each time?
	 */
	struct bigint *a, *b, *m, *res;
	int words = 8;

	bi_init(&a, words);
	bi_set(a, 7u);
	bi_init(&b, words);
	bi_set(b, 11u);
	bi_init(&m, words);
	bi_set(m, 29u);

	cfg.a = a;
	cfg.b = b;
	cfg.m = m;
	cfg.words = words;

	printf("testing init_will_rng\n");
	init_will_rng(&cfg, seed, &state);

	printf("testing will_rng_next\n");
	for(int i = 0; i < 50; i++){
		will_rng_next(&state, &res);

		bi_printf(res);
		printf("\n");
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
void tests(){
	printf("Testing bigint\n");
	test_bigint();

	printf("Testing rng\n");
	test_rng();

	printf("Tests completed!\n");

//	test_rsa();
}

int main(){
	tests();
}
