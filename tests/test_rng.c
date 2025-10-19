#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <crypto_core/rsa.h>
#include <rng/chacha.h>
#include <rng/rng.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "test_suites.h"

void test_rng(void) {
    uint32_t words = 32;

    MPI res;
    will_rng_init(12345678u);

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

    uint32_t *a, *b;

    a = malloc(16 * sizeof(uint32_t));

    for (int i = 0; i < 5; i++) {
        b = malloc(16 * sizeof(uint32_t));

        if (i == 0) {
            for (int j = 0; j < 16; j++)
                a[j] = j;
        }

        chacha_block(b, a);

        free(a);
        a = b;
    }
    free(b);
}

CU_pSuite register_rng_tests(void) {
    CU_pSuite suite = CU_add_suite("RNG_Suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "rng", test_rng);
    CU_add_test(suite, "chacha", test_chacha);

    return suite;
}
