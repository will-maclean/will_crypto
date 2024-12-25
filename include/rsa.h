#ifndef __rsa
#define __rsa

#include <stdio.h>
#include <stdbool.h>
#include "bigint.h"

#define RSA_DEFAULT_E 65537

struct rsa_public_token {
	struct bigint *e;
	struct bigint *n;
};

struct rsa_private_token {
	struct bigint *d;
	struct bigint *n;
};

struct rsa_state {
	struct bigint *p;
	struct bigint *q;
};

static void load_new_primes(struct rsa_state *new_state, int seed);

/*
 * Assumes bez_x and bez_y are NOT set
 */
int lcm_ext_euc(struct bigint *a, struct bigint *b, struct bigint *bez_x,
		struct bigint *bez_y);

static int calc_lambda_n_d(struct bigint *p, struct bigint *q,
			struct bigint *lambda_n, struct bigint* d);

static int gen_e(struct bigint *res);

void gen_pub_priv_keys(long seed, struct rsa_public_token *pub,
		       struct rsa_private_token *priv);

bool primality_test(struct bigint *x);
#endif
