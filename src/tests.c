#include "bigint.h"
#include "chacha.h"
#include "primality.h"
#include "rng.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int successes = 0;
static int failures = 0;

void assert(bool result, char *failure_msg) {
    if (result) {
        successes++;
    } else {
        failures++;
        printf("FAILURE: %s\n", failure_msg);
    }
}

void test_bigint_math_proper(void) {
    // start with one word tests

    int words = 1;
    MPI a, b, res, expected_res;

    a = bi_init(words);
    b = bi_init(words);
    expected_res = bi_init(words);

    bi_set(a, 5u);
    bi_set(b, 2u);

    assert(!bi_even(a), "bi_even for an odd number failing");
    assert(bi_even(b), "bi_even for an even number failing");

    // comparisons
    printf("Testing comparisons...\n");
    assert(bi_gt(a, b), "a>b (true) failing, single-word");
    assert(!bi_gt(b, a), "b>a (false) failing, single-word");
    assert(!bi_lt(a, b), "a<b (false) failing, single-word");
    assert(bi_lt(b, a), "b<a (true) failing, single-word");
    assert(bi_ge(a, b), "a>=b (true) failing, single-word");
    assert(!bi_ge(b, a), "b>=a (false) failing, single-word");
    assert(!bi_le(a, b), "a<=b (false) failing, single-word");
    assert(bi_le(b, a), "b<=a (true) failing, single-word");

    bi_free(a);
    bi_free(b);
    a = bi_init(2);
    b = bi_init(2);
    a->data[1] = 1;
    a->data[0] = 5;
    b->data[1] = 1;
    b->data[0] = 2;

    assert(bi_gt(a, b), "a>b (true) failing, multi-word");
    assert(!bi_gt(b, a), "b>a (false) failing, multi-word");
    assert(!bi_lt(a, b), "a<b (false) failing, multi-word");
    assert(bi_lt(b, a), "b<a (true) failing, multi-word");
    assert(bi_ge(a, b), "a>=b (true) failing, nulti-word");
    assert(!bi_ge(b, a), "b>=a (false) failing, multi-word");
    assert(!bi_le(a, b), "a<=b (false) failing, multi-word");
    assert(bi_le(b, a), "b<=a (true) failing, multi-word");

    bi_free(a);
    bi_free(b);
    a = bi_init(1);
    b = bi_init(1);
    bi_set(a, 5u);
    bi_set(b, 2u);

    printf("Testing arithmetic...\n");
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
    bool passed = bi_eq(res, expected_res);
    assert(passed, "bigint 1-word multiplication");
    if (!passed) {
        printf("a, b, res:\n");
        bi_printf(a);
        printf("\n");
        bi_printf(b);
        printf("\n");
        bi_printf(res);
        printf("\n");
    }
    bi_free(res);

    // integer division
    bi_set(expected_res, 2u);
    res = bi_eucl_div(a, b);
    assert(bi_eq(res, expected_res), "bigint 1-word euclidian division");
    bi_free(res);

    // integer division, multi-word
    MPI a_euc = bi_init(2);
    MPI b_euc = bi_init(1);
    a_euc->data[1] = 1;
    a_euc->data[0] = 2;
    b_euc->data[0] = 2;
    bi_set(expected_res, 0x80000001);
    res = bi_eucl_div(a_euc, b_euc);
    passed = bi_eq(res, expected_res);

    assert(passed, "bigint 2-word euclidian division");
    if (!passed) {
        printf("a, b, res:\n");
        bi_printf(a_euc);
        printf("\n");
        bi_printf(b_euc);
        printf("\n");
        bi_printf(res);
        printf("\n");
    }
    bi_free(res);

    // modulo
    bi_set(expected_res, 1u);
    res = bi_mod(a, b);
    assert(bi_eq(res, expected_res), "bigint 1-word modulo");
    bi_free(res);

    // inc
    bi_set(expected_res, 6u);
    bi_set(a, 5u);
    bi_inc(a);
    assert(bi_eq(a, expected_res), "bigint 1-word increment");
    bi_set(a, 5u);

    // dec
    bi_set(expected_res, 4u);
    bi_set(a, 5u);
    bi_dec(a);
    assert(bi_eq(a, expected_res), "bigint 1-word decrement");
    bi_set(a, 5u);

    bi_free(expected_res);
    res = bi_init(2);
    bi_free(a);
    a = bi_init(2);
    a->data[1] = 5;
    a->data[0] = 5;
    expected_res = bi_init(2);
    expected_res->data[1] = 5;
    expected_res->data[0] = 4;
    bi_dec(a);
    passed = bi_eq(a, expected_res);
    assert(passed, "bigint 2-word decrement");
    if (!passed) {
        printf("a, expected_res:\n");
        bi_printf(a);
        printf("\n");
        bi_printf(expected_res);
        printf("\n");
    }

    bi_free(a);
    bi_free(expected_res);

    a = bi_init(1);
    expected_res = bi_init(1);
    bi_set(a, 5u);

    // shift left
    bi_free(b);
    bi_set(a, 2u);
    bi_set(expected_res, 8u);
    b = bi_shift_left(a, 2u);
    assert(bi_eq(b, expected_res), "bigint 1-word shift left");
    bi_set(a, 5u);
    bi_set(b, 3u);

    // shift right
    bi_free(b);
    bi_set(a, 5u);
    bi_set(expected_res, 2u);
    b = bi_shift_right(a, 1u);
    assert(bi_eq(b, expected_res), "bigint 1-word shift right");
    bi_set(a, 5u);
    bi_set(b, 3u);

    // mod exp
    // (5 ^ 2) % 4 = 1
    MPI mod = bi_init(words);
    bi_set(mod, 4u);
    bi_set(expected_res, 1u);
    res = bi_mod_exp(a, b, mod);
    assert(bi_eq(res, expected_res), "bigint 1-word modular exponentiation");
    bi_free(res);
    bi_free(mod);
}

void test_bigint(void) {
    // test create and free
    int test_words = 2;

    MPI x, y, z;

    x = bi_init(4);
    bi_squeeze(x);
    assert(x->words == 1, "squeeze on zero failed");
    bi_free(x);

    x = bi_init(4);
    x->data[1] = 42u;
    bi_squeeze(x);
    assert(x->words == 2, "squeeze on non-zero failed");
    bi_free(x);

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

    if (!bi_eq(x, y)) {
        printf("testing set: x and y are not equal");
        exit(1);
    }

    bi_set(y, 10ul);
    if (bi_eq(x, y)) {
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

    for (int i = 0; i < 3; i++) {
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
    printf("x=");
    bi_printf(x);
    printf(", y=");
    bi_printf(y);
    printf("\n");
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

void test_rng(void) {
    struct will_rng_cfg cfg;
    uint32_t seed = 12345678u;
    int words = 32;
    cfg.words = words;

    MPI res;
    printf("testing init_will_rng\n");
    init_will_rng(&cfg, seed);

    printf("testing will_rng_next\n");
    for (int i = 0; i < 5; i++) {
        res = will_rng_next(words);

        bi_printf(res);
        bi_free(res);
        printf("\n");
    }

    // See how many rng gens we can get done in a second
    uint64_t counter = 0;
    clock_t start = clock();
    while (clock() - start < CLOCKS_PER_SEC) {
        res = will_rng_next(words);
        bi_free(res);
        counter++;
    }

    printf("In one sec, for %d-word numbers, generated %lu nums\n", words,
           counter);
}

void test_chacha(void) {
    printf("Testing chacha, with zero as init\n");

    uint32_t *a, *b;

    a = malloc(16 * sizeof(uint32_t));

    for (int i = 0; i < 5; i++) {
        b = malloc(16 * sizeof(uint32_t));

        if (i == 0) {
            for (int j = 0; j < 16; j++)
                a[j] = j;
        }

        chacha_block(b, a);

        printf("iter %d\n", i);
        for (int j = 0; j < 16; j++) {
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

void test_primality(void) {
    printf("Starting primality tests\n");

    MPI a = bi_init(1);

    bi_set(a, 23u);
    struct mr_sd sd = miller_rabin_sd(a);
    assert(bi_eq_val(sd.s, 1u), "Miller-Rabin s value (a=23)");
    assert(bi_eq_val(sd.d, 11u), "Miller-Rabin d (a=23)");
    bi_free(sd.s);
    bi_free(sd.d);

    bi_set(a, 97u);
    sd = miller_rabin_sd(a);
    assert(bi_eq_val(sd.s, 5u), "Miller-Rabin s value (a=97)");
    assert(bi_eq_val(sd.d, 3u), "Miller-Rabin d (a=97)");
    bi_free(sd.s);
    bi_free(sd.d);

    bi_set(a, 341u);
    sd = miller_rabin_sd(a);
    assert(bi_eq_val(sd.s, 2u), "Miller-Rabin s value (a=341)");
    assert(bi_eq_val(sd.d, 85u), "Miller-Rabin d (a=341)");
    bi_free(sd.s);
    bi_free(sd.d);

    bi_free(a);
    a = bi_init(2);
    a->data[1] = 1;
    a->data[0] = 1;
    sd = miller_rabin_sd(a);
    assert(bi_eq_val(sd.s, 32u), "Miller-Rabin s value (a=0x10001)");
    assert(bi_eq_val(sd.d, 1), "Miller-Rabin d (a=0x10001)");
    bi_free(sd.s);
    bi_free(sd.d);

    bi_free(a);
    a = bi_init(1);
    bi_set(a, 0xFFFFFFFB); // obviously prime
    assert(miller_rabin(a, 5), "Miller-Rabin primality test (prime a=0xFFFFFFFB)");
    bi_free(a);
    bi_set(a, 0xFFFFFFF0); // ob
    assert(!miller_rabin(a, 5), "Miller-Rabin primality test (composite a=0xFFFFFFF0)");
    bi_free(a);

    int words = 16;
    MPI test_prime = will_rng_next(words);

    printf("rng'd a big word: ");
    bi_printf(test_prime);
    printf("\n");

    bool is_prime = miller_rabin(test_prime, 1000);

    if (is_prime) {
        printf("The number is probably prime\n\ns");
    } else {
        printf("The number is composite\n\n");
    }

    // this one takes a while, so if we've had errors elsewhere, don't run it
    if (failures == 0) {
        MPI generated_prime = gen_prime(words);
        if (generated_prime) {
            printf("Generated prime:\n");
            printf("\n");
            bi_printf(generated_prime);
        } else {
            printf("Failed to generate prime\n");
        }
    } else {
        printf("Skipping gen_prime test due to earlier failures\n");
    }
}

void KISS(void) {

    // integer division, multi-word
    MPI a_euc = bi_init(2);
    MPI b_euc = bi_init(1);
    MPI expected_res = bi_init(1);
    a_euc->data[1] = 1;
    a_euc->data[0] = 2;
    b_euc->data[0] = 2;
    bi_set(expected_res, 0u);
    MPI res = bi_eucl_div(a_euc, b_euc);
    assert(bi_eq(res, expected_res), "bigint 2-word euclidian division");
    printf("Res: ");
    bi_printf(res);
    bi_free(res);
}

void tests(void) {
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
    printf("Tests: %d. Passes: %d. Failures: %d\n", successes + failures,
           successes, failures);

    //	test_rsa();
}

int main(void) { tests(); }
