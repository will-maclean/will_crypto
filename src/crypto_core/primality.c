#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <rng/rng.h>
#include <stdio.h>
#include <stdlib.h>

struct mr_sd miller_rabin_sd(MPI n) {
    MPI s = bi_init_like(n);
    MPI tmp_two = bi_init_like(n);
    bi_set(tmp_two, 2u);
    MPI tmp_zero = bi_init_like(n);
    bi_set(tmp_zero, 0u);

    MPI d = bi_init_and_copy(n);
    bi_dec(d);

    while (bi_even(d)) {
        MPI tmp = bi_eucl_div(d, tmp_two);
        bi_copy(tmp, d);
        bi_free(tmp);
        bi_inc(s);
    }

    bi_free(tmp_two);
    bi_free(tmp_zero);

    return (struct mr_sd){
        .s = s,
        .d = d,
    };
}

MPI miller_rabin_randn(MPI n) {
    MPI n_minus_two = bi_init_and_copy(n);
    bi_dec(n_minus_two);
    bi_dec(n_minus_two);

    MPI tmp_two = bi_init_like(n);
    bi_set(tmp_two, 2u);

    int max_iters = 100000;
    MPI a;

    for (int i = 0; i < max_iters; i++) {
        // have to do some funniness to get random numbers of the
        // correct length
        a = will_rng_next(n->words);

        if (bi_gt(a, tmp_two) && bi_lt(a, n_minus_two)) {
            bi_free(n_minus_two);
            bi_free(tmp_two);
            return a;
        }

        bi_free(a);
    }

    printf(
        "WARNING: miller_rabin_randn failed to find a suitable rand for n:\n");
    bi_print(n);
    printf("\n");

    bi_free(n_minus_two);
    bi_free(tmp_two);

    return NULL;
}

bool __miller_rabin_inner_check(MPI n, MPI a, struct mr_sd sd) {
    MPI tmp_two = bi_init_like(n);
    bi_set(tmp_two, 2u);

    MPI tmp_n_minus_one = bi_init_and_copy(n);
    bi_dec(tmp_n_minus_one);

    MPI x = bi_mod_exp(a, sd.d, n);

    MPI s_ = bi_init_like(sd.d);
    bi_set(s_, 0u);
    for (; bi_lt(s_, sd.s); bi_inc(s_)) {
        MPI y = bi_mod_exp(x, tmp_two, n);

        if (bi_eq_val(y, 1u) && !bi_eq_val(x, 1u) &&
            !bi_eq(x, tmp_n_minus_one)) {
            // nontrivial square root of 1 modulo n
            bi_free(tmp_two);
            bi_free(tmp_n_minus_one);
            bi_free(x);
            bi_free(s_);
            return false;
        }

        bi_copy(y, x);
        bi_free(y);
    }

    bi_free(tmp_two);
    bi_free(tmp_n_minus_one);
    bi_free(s_);
    bool res = bi_eq_val(x, 1);
    bi_free(x);
    return res;
}

bool miller_rabin(MPI n, int k) {
    /* Before starting the test, we must assert:
     * 1. n > 2
     * 2. n is odd
     */
    // printf("Starting Miller Rabin\n");

    if (bi_eq_val(n, 1u) || bi_eq_val(n, 2u))
        // 1 and 2 are prime
        return true;

    if (bi_even(n))
        // x is even, so therefore is not prime
        return false;

    // printf("MR: initial assertions passed\n");

    // Assertions have passed, so we can now start
    // the primality test

    // We need to find s and d s.t. n-1 = 2^s * d
    // We factor out powers of 2 from n-1 until the
    // result is no longer divisible by 2
    struct mr_sd sd = miller_rabin_sd(n);

    // printf("MR: generated s, d. s=\n");
    // bi_printf(s);
    // printf("\nd=\n");
    // bi_printf(d);
    // printf("\n");

    for (int k_ = 0; k_ < k; k_++) {
        // Sets a with a random number betwee 2 and n-2
        MPI a = miller_rabin_randn(n);

        if (a == NULL) {
            bi_free(sd.s);
            bi_free(sd.d);

            printf("ERROR: Miller-Rabin RNG failed (trial %d/%d)\n", k_, k);
            exit(1);
        }

        if (!__miller_rabin_inner_check(n, a, sd)) {
            bi_free(sd.s);
            bi_free(sd.d);
            bi_free(a);
            return false;
        }

        bi_free(a);
    }

    bi_free(sd.s);
    bi_free(sd.d);
    return true;
}

MPI gen_prime(int words) {
    int max_tries = 10000;
    int mr_k = 20;
    MPI res;
    int counter = 0;

    while (counter < max_tries) {
        res = will_rng_next(words);
        res->data[0] |= 1u; // make sure all the generated numbers are odd

        if (miller_rabin(res, mr_k)) {
            break;
        }

        bi_free(res);
        counter++;
    }

    if (counter >= max_tries) {
        return NULL;
    }

    return res;
}
