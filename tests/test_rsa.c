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

void test_rsa(void) {
    uint32_t seed = 1234;

    struct rsa_public_token pub;
    struct rsa_private_token priv;

    gen_pub_priv_keys(seed, &pub, &priv, RSA_MODE_512);

    printf("Generated public and private keys for RSA (512 bit key)\nn:\n");
    bi_print(pub.n);
    printf("\ne:\n");
    bi_print(pub.e);
    printf("\nd:\n");
    bi_print(priv.d);
    printf("\n");

    bi_free(pub.e);
    bi_free(pub.n);
    bi_free(priv.d);
    bi_free(priv.n);
}

void test_ext_euc(void) {
    // clang-format off
    uint32_t tests[] = {
        1, 12,             1, 36,             1, 12,
        1, 0x00000000,             1, 0x00000000,             1, 0x00000000,
        1, 0x00000000,             1, 0x00000024,             1, 0x00000024,
        1, 0x00000024,             1, 0x00000000,             1, 0x00000024,
        1, 0x0000000C,             1, 0x00000024,             1, 0x0000000C,
        1, 0x00000007,             1, 0x00000014,             1, 0x00000001,
        1, 0xFFFFFFFF,             1, 0x00000002,             1, 0x00000001,
        1, 0x00010000,             1, 0x00100000,             1, 0x00010000,
        2, 0x00000000, 0x00000001, 1, 0x00000003,             1, 0x00000001,
        3, 0x00000000, 0x00000000, 0x00000001,
           2, 0x00010000, 0x00000000,
           2, 0x00010000, 0x00000000,
        3, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
           1, 0xFFFFFFFF,
           1, 0xFFFFFFFF,
        2, 0x89ABCDEF, 0x01234567,
           2, 0x89ABCDEF, 0x01234567,
           2, 0x89ABCDEF, 0x01234567,
        2, 0x9ABCDEF0, 0x12345678,
           2, 0x00010000, 0x00000000,
           1, 0x00000010,
    };
    // clang-format on

    uint32_t curr_pos = 0;
    uint32_t test = 0;
    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos++];
        MPI a = bi_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a->data[j] = tests[curr_pos++];

        uint32_t b_words = tests[curr_pos++];
        MPI b = bi_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b->data[j] = tests[curr_pos++];

        uint32_t eg_words = tests[curr_pos++];
        MPI expected_gcd = bi_init(eg_words);
        for (uint32_t j = 0; j < eg_words; j++)
            expected_gcd->data[j] = tests[curr_pos++];

        struct ext_euc_res res = ext_euc(a, b);

        bool gcd_ok = bi_eq(res.gcd, expected_gcd);
        CU_ASSERT(gcd_ok);
        if (!gcd_ok) {
            printf("case=%d\ba=", test);
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\nexpected gcd=");
            bi_print(expected_gcd);
            printf("\nreturned gcd=");
            bi_print(res.gcd);
            printf("\nBezout x=");
            bi_print(res.bez_x);
            printf("\nBezout y=");
            bi_print(res.bez_y);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(expected_gcd);
        bi_free(res.bez_x);
        bi_free(res.bez_y);
        bi_free(res.gcd);
        test++;
    }
}

CU_pSuite register_rsa_tests(void) {
    CU_pSuite suite = CU_add_suite("RSA_Suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "rsa", test_rsa);
    CU_add_test(suite, "ext_euc", test_ext_euc);

    return suite;
}
