#include <stdio.h>
#include "rsa.h"
#include "bigint.h"

static void load_new_primes(struct rsa_state *new_state, int seed)
{
	//TODO (you reckon?)

	bi_set(new_state->p, 11);
	bi_set(new_state->q, 13);
}

/*
 * Assumes bez_x and bez_y are NOT set
 */
int lcm_ext_euc(struct bigint *a, struct bigint *b, struct bigint *bez_x,
		struct bigint *bez_y)
{
	/* TODO: verify that we're calculating the correct things for lambda_n
	 * and d
	 */
	struct bigint r, s, t, old_r, old_s, old_t, quotient, tmp1, tmp2;

	// Setup
	bi_init_and_copy(a, &old_r);
	bi_init_and_copy(b, &r);
	bi_init_like(&s, a);
	bi_set(&s, 0ul);
	bi_init_like(&old_s, a);
	bi_set(&old_s, 1ul);
	bi_init_like(&t, a);
	bi_set(&t, 1ul);
	bi_init_like(&old_t, a);
	bi_set(&old_t, 0ul);

	bi_init_like(&quotient, a);
	bi_init_like(&tmp1, a);
	bi_init_like(&tmp2, a);

	while (!bi_eq_val(&r, 0ul)){
		bi_eucl_div(&old_r, &r, &quotient);

		bi_copy(&r, &tmp1);
		bi_mul(&quotient, &tmp1, &tmp2)
		bi_sub(&old_r, &tmp2, &r)
		bi_copy(&tmp1, &old_r);

		bi_copy(&s, &tmp1);
		bi_mul(&quotient, &tmp1, &tmp2)
		bi_sub(&old_s, &tmp2, &s)
		bi_copy(&tmp1, &old_s);

		bi_copy(&t, &tmp1);
		bi_mul(&quotient, &tmp1, &tmp2)
		bi_sub(&old_t, &tmp2, &t)
		bi_copy(&tmp1, &old_t);
	}

	/* The Bézout coefficients are what we're here for. There's lots
	 * of other information that can be extracted from old_r, t, and s,
	 * but we don't care about it.
	 */

	bi_init_and_copy(&old_s, bez_x);
	bi_init_and_copy(&old_t, bez_y);
}


static int calc_lambda_n_d(struct bigint *p, struct bigint *q,
			struct bigint *lambda_n, struct bigint* d)
{
	lcm_euclidean(bi_sub(p, 1), bi_sub(q, 1), lambda_n, d);
}

static int gen_e(struct bigint *res)
{
	// Could flesh this out to be more complicated, but I think there
	// are going to be bigger problems before I start worrying about
	// alternate implementations of e
	bi_set(res, RSA_DEFAULT_E);
}

void gen_pub_priv_keys(long seed, struct rsa_public_token *pub,
		       struct rsa_private_token *priv)
{

	struct rsa_state state;
	struct bigint *n, *lambda_n, *e, *d;

	load_new_primes(&state, seed);

	bi_mul(state.p, state.q, n);

	calc_lambda_n_d(state.p, state.q, lambda_n, d);

	e = gen_e(e);

	pub->e = e;
	pub->n = n;

	priv->d = d;
	priv->n = n;
}

bool primality_test(struct bigint *x)
{
	//TODO
	

	return false;	
}
