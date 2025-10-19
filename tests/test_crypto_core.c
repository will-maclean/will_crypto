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

void test_primality(void) {

    MPI a = bi_init(1);

    bi_set(a, 23u);
    struct mr_sd sd = miller_rabin_sd(a);
    CU_ASSERT(bi_eq_val(sd.s, 1u));
    CU_ASSERT(bi_eq_val(sd.d, 11u));
    bi_free(sd.s);
    bi_free(sd.d);

    bi_set(a, 97u);
    sd = miller_rabin_sd(a);
    CU_ASSERT(bi_eq_val(sd.s, 5u));
    CU_ASSERT(bi_eq_val(sd.d, 3u));
    bi_free(sd.s);
    bi_free(sd.d);

    bi_set(a, 341u);
    sd = miller_rabin_sd(a);
    CU_ASSERT(bi_eq_val(sd.s, 2u));
    CU_ASSERT(bi_eq_val(sd.d, 85u));
    bi_free(sd.s);
    bi_free(sd.d);

    bi_free(a);
    a = bi_init(2);
    a->data[1] = 1;
    a->data[0] = 1;
    sd = miller_rabin_sd(a);
    CU_ASSERT(bi_eq_val(sd.s, 32u));
    CU_ASSERT(bi_eq_val(sd.d, 1));
    bi_free(sd.s);
    bi_free(sd.d);

    bi_free(a);
    a = bi_init(1);
    bi_set(a, 0xFFFFFFFB); // obviously prime
    CU_ASSERT(miller_rabin(a, 5));
    bi_set(a, 0xFFFFFFF0); // ob
    CU_ASSERT(!miller_rabin(a, 5));
    bi_free(a);

    // simple check for miller rabin inner loop check - if this is false
    // for a prime number, then the whole thing is broken
    MPI n = bi_init(1);
    bi_set(n, 23u);
    sd = miller_rabin_sd(n);
    a = bi_init(1);

    for (uint32_t i = 2; i < 21; i++) {
        bi_set(a, i);
        CU_ASSERT(__miller_rabin_inner_check(n, a, sd));
    }
    bi_free(a);
    bi_free(n);
    bi_free(sd.s);
    bi_free(sd.d);

    clock_t start = clock();
    MPI generated_prime = gen_prime(32);
    clock_t ticks = clock() - start;

    printf("Prime number gen took %f milliseconds\n",
           1000 * (float)ticks / (float)CLOCKS_PER_SEC);

    if (generated_prime) {
        printf("Generated prime:\n");
        printf("\n");
        bi_print(generated_prime);
    } else {
        printf("Failed to generate prime\n");
    }

    bi_free(generated_prime);
}

CU_pSuite register_crypto_core_tests(void) {
    CU_pSuite suite = CU_add_suite("CRYPTO_CORE_Suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "primality", test_primality);

    return suite;
}
