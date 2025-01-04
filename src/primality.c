#include <stdlib.h>
#include <stdio.h>
#include "primality.h"
#include "rng.h"

struct mr_sd{
	MPI s;
	MPI d;
};

struct mr_sd miller_rabin_sd(MPI n)
{
	MPI s = bi_init_like(n);
	MPI tmp_two = bi_init_like(n);
	bi_set(tmp_two, 2u);
	MPI tmp_zero = bi_init_like(n);
	bi_set(tmp_zero, 0u);

	MPI d = bi_init_and_copy(n);
	bi_dec(d);

	while(bi_even(d)){
		MPI tmp = bi_eucl_div(d, tmp_two);
		bi_copy(tmp, d);
		bi_free(tmp);

		bi_inc(s);
	}

	//TODO: assert d * 2 ^ s = n - 1
	bi_squeeze(s);
	bi_squeeze(d);

	return (struct mr_sd){
		.s = s,
		.d = d,
	};
}

MPI miller_rabin_randn(MPI n)
{
	MPI n_minus_two = bi_init_and_copy(n);
	bi_dec(n_minus_two);
	bi_dec(n_minus_two);

	MPI tmp_two = bi_init_like(n);
	bi_set(tmp_two, 2u);

	int max_iters = 1000;
	MPI a, tmp2, tmp3;
	for(int i = 0; i < max_iters; i++){	
		// have to do some funniness to get random numbers of the
		// correct length
		a = will_rng_next(n->words);

		if(bi_gt(a, tmp_two) && bi_lt(a, n_minus_two)){
			break;
		}
	}

	return a;
}

bool miller_rabin(MPI n, int k)
{
	/* Before starting the test, we must assert:
	 * 1. n > 2
	 * 2. n is odd
	 */
	MPI tmp1 = bi_init_like(n);
	bi_set(tmp1, 2u);

	if(bi_le(n, tmp1))
		// 1 and 2 are prime
		return true;
	
	if(bi_even(n))
		// x is even, so therefore is not prime
		return false;

	bi_free(tmp1);

	// Assertions have passed, so we can now start
	// the primality test
	
	// We need to find s and d s.t. n-1 = 2^s * d
	// We factor out powers of 2 from n-1 until the
	// result is no longer divisible by 2
	struct mr_sd sd = miller_rabin_sd(n);
	MPI s = sd.s;
	MPI d = sd.d;

	MPI tmp_two = bi_init_like(n);
	MPI tmp_one = bi_init_like(n);
	MPI tmp_n_minus_one = bi_init_and_copy(n);
	bi_dec(tmp_n_minus_one);
	bi_set(tmp_one, 1u);
	bi_set(tmp_two, 2u);

	for(int k_ = 0; k_ < k; k_++){
		// Sets a with a random number betwee 2 and n-2
		MPI a = miller_rabin_randn(n);
		MPI x = bi_mod_exp(a, d, n);
		
		MPI s_ = bi_init_like(s);
		bi_set(s_, 0u);
		for(; bi_lt(s_, s); bi_inc(s_)){
			MPI y = bi_mod_exp(x, tmp_two, n);
			
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

MPI gen_prime(int words)
{
	int max_tries = 10000;
	int mr_k = 1000;
	MPI res;
	int counter = 0;
	
	while(counter < max_tries) {
		res = will_rng_next(words);
		res->data[0] |= 1u;	// make sure all the generated numbers are odd

		if(miller_rabin(res, mr_k)){
			break;
		} else {
			bi_free(res);
		}

		counter ++;
	}

	if(counter >= max_tries){
		// error handling
		printf("gen_prime failed!\n");
		return NULL;
	}

	return res;
}