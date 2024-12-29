#include <stdlib.h>
#include <stdio.h>
#include "primality.h"
#include "rng.h"

void miller_rabin_sd(struct bigint *n, struct bigint **s, struct bigint **d)
{
	struct bigint *mod_res, *n_tmp, *tmp_two, *tmp_zero;

	bi_init_like(s, n);
	bi_init_like(&tmp_two, n);
	bi_set(tmp_two, 2u);
	bi_init_like(&tmp_zero, n);
	bi_set(tmp_two, 0u);

	bi_init_and_copy(n, &n_tmp);
       	bi_dec(n_tmp);
	
	bi_mod(n_tmp, tmp_two, &mod_res);
	while(bi_eq(mod_res, tmp_zero)){
		struct bigint *tmp;
		bi_eucl_div(n_tmp, tmp_two, &tmp);
		bi_copy(tmp, n_tmp);
		bi_free(tmp);

		bi_free(mod_res);
		bi_mod(n_tmp, tmp_two, &mod_res);

		bi_inc(*s);
	}

	bi_mul(n_tmp, tmp_two, d);
}

void miller_rabin_randn(struct bigint *n, struct bigint **a)
{
	int rng_words = get_rng_words();
	
	if(n->words % rng_words != 0){
		// ERROR
		printf("mill_rabin_randn: n->words %% rng_words != 0");
		exit(1);
	}
	int rng_per_n = n->words / rng_words;

	struct bigint *n_minus_two, *tmp_two;
	bi_init_and_copy(n, &n_minus_two);
	bi_dec(n_minus_two);
	bi_dec(n_minus_two);

	bi_init_like(&tmp_two, n);
	bi_set(tmp_two, 2u);

	int max_iters = 1000;
	struct bigint *tmp2, *tmp3;
	for(int i = 0; i < max_iters; i++){	
		// have to do some funniness to get random numbers of the
		// correct length
		will_rng_next(a);
		for(int j = 0; j < rng_per_n; j++){
			will_rng_next(&tmp2);
			bi_init_and_copy(*a, &tmp3);
			bi_free(*a);
			bi_concat(tmp3, tmp2, a);
			bi_free(tmp2);
			bi_free(tmp3);
		}

		if(bi_gt(*a, tmp_two) && bi_lt(*a, n_minus_two)){
			break;
		}
	}
}

bool miller_rabin(struct bigint *n, int k)
{
	/* Before starting the test, we must assert:
	 * 1. n > 2
	 * 2. n is odd
	 */
	struct bigint *tmp1, *tmp2;
	bi_init_like(&tmp1, n);
	bi_set(tmp1, 2u);

	if(bi_le(n, tmp1))
		// 1 and 2 are prime
		return true;
	
	bi_mod(n, tmp1, &tmp2);
	bi_set(tmp1, 1u);
	if(!bi_eq(tmp1, tmp2))
		// x is even, so therefore is not prime
		return false;

	bi_free(tmp1);
	bi_free(tmp2);

	// Assertions have passed, so we can now start
	// the primality test
	
	// We need to find s and d s.t. n-1 = 2^s * d
	// We factor out powers of 2 from n-1 until the
	// result is no longer divisible by 2
	struct bigint *s, *d;
	miller_rabin_sd(n, &s, &d);	

	struct bigint *a, *x, *y, *tmp_one, *tmp_n_minus_one, *tmp_two;
	bi_init_like(&tmp_two, n);
	bi_init_like(&tmp_one, n);
	bi_init_and_copy(n, &tmp_n_minus_one);
	bi_dec(tmp_n_minus_one);
	bi_set(tmp_one, 1u);
	bi_set(tmp_two, 2u);
	for(int k_ = 0; k_ < k; k_++){
		// Sets a with a random number betwee 2 and n-2
		miller_rabin_randn(n, &a);
		bi_mod_exp(a, d, n, &x);
		
		struct bigint *s_;
		bi_init_like(&s_, s);
		bi_set(s_, 0u);
		for(; bi_lt(s_, s); bi_inc(s_)){
			bi_mod_exp(x, tmp_two, n, &y);
			
			if(
				bi_eq(y, tmp_one) &&
				!bi_eq(x, tmp_one) &&
				!bi_eq(x, tmp_n_minus_one)		
			){
				// nontrivial square root of 1 modulo n
				return false;
			}

			bi_copy(y, x);
			bi_free(y);
		}

		if(!bi_eq(x, tmp_one))
			return false;

		bi_free(x);
		bi_free(a);
	}

	return true;
}	
