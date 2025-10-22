#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <crypto_core/rsa.h>
#include <iso646.h>
#include <rng/chacha.h>
#include <rng/rng.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "test_suites.h"

void test_rsa_keygen(void) {
    uint32_t seed = 1234;

    rsa_public_token_t pub;
    rsa_private_token_t priv;

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



void test_calc_lambda_n_d(void) {
    //   p_words, p[0].., q_words, q[0].., e_words, e[0].., lambda_words,
    //   lambda[0].., d_words, d[0]..
    // clang-format off
    uint32_t tests[] = {
        1, 0x00000005,   1, 0x0000000B,   1, 0x00000003,   1, 0x00000014,   1, 0x00000007,
        1, 0x00000011,   1, 0x00000017,   1, 0x00000003,   1, 0x000000B0,   1, 0x0000003B,
        1, 0x0000000B,   1, 0x0000000D,   1, 0x0000000B,   1, 0x0000003C,   1, 0x0000000B,
    };
    // clang-format on

    uint32_t curr = 0;
    uint32_t case_idx = 0;

    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t p_words = tests[curr++];
        MPI p = bi_init(p_words);
        for (uint32_t j = 0; j < p_words; j++)
            p->data[j] = tests[curr++];

        uint32_t q_words = tests[curr++];
        MPI q = bi_init(q_words);
        for (uint32_t j = 0; j < q_words; j++)
            q->data[j] = tests[curr++];

        uint32_t e_words = tests[curr++];
        MPI e = bi_init(e_words);
        for (uint32_t j = 0; j < e_words; j++)
            e->data[j] = tests[curr++];

        uint32_t lambda_words = tests[curr++];
        MPI expected_lambda = bi_init(lambda_words);
        for (uint32_t j = 0; j < lambda_words; j++)
            expected_lambda->data[j] = tests[curr++];

        uint32_t d_words = tests[curr++];
        MPI expected_d = bi_init(d_words);
        for (uint32_t j = 0; j < d_words; j++)
            expected_d->data[j] = tests[curr++];

        lambda_n_d_res_t res = calc_lambda_n_d(p, q, e);

        bool lambda_ok = bi_eq(res.lambda_n, expected_lambda);
        CU_ASSERT(lambda_ok);
        if (!lambda_ok) {
            printf("case=%u lambda mismatch\np=", case_idx);
            bi_print(p);
            printf("\nq=");
            bi_print(q);
            printf("\ne=");
            bi_print(e);
            printf("\nexpected lambda(n)=");
            bi_print(expected_lambda);
            printf("\ncalc lambda(n)=");
            bi_print(res.lambda_n);
            printf("\n\n");
        }

        bool d_ok = bi_eq(res.d, expected_d);
        CU_ASSERT(d_ok);
        if (!d_ok) {
            printf("case=%u d mismatch\np=", case_idx);
            bi_print(p);
            printf("\nq=");
            bi_print(q);
            printf("\ne=");
            bi_print(e);
            printf("\nexpected d=");
            bi_print(expected_d);
            printf("\ncalc d=");
            bi_print(res.d);
            printf("\n\n");
        }

        bi_free(p);
        bi_free(q);
        bi_free(e);
        bi_free(expected_lambda);
        bi_free(expected_d);
        bi_free(res.lambda_n);
        bi_free(res.d);
        case_idx++;
    }
}

void test_rsa_encrypt_decrypt_cases(void) {
    // clang-format off
    uint32_t tests[] = {
        // n words, ...n, e words, ...e, d words, ...d, m words, ...m, c words, ...c,
        1, 3233, 1, 17, 1, 413, 1, 65, 1, 2790,
    };
    // clang-format on

    uint32_t curr_pos = 0;

    MPI n, e, d, m, c, calc_c, calc_m;
    rsa_public_token_t pub;
    rsa_private_token_t priv;

    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t n_words = tests[curr_pos];
        n = bi_init(n_words);
        curr_pos++;

        for (uint32_t i = 0; i < n_words; i++) {
            n->data[i] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t e_words = tests[curr_pos];
        e = bi_init(e_words);
        curr_pos++;

        for (uint32_t i = 0; i < e_words; i++) {
            e->data[i] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t d_words = tests[curr_pos];
        d = bi_init(d_words);
        curr_pos++;

        for (uint32_t i = 0; i < d_words; i++) {
            d->data[i] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t m_words = tests[curr_pos];
        m = bi_init(m_words);
        curr_pos++;

        for (uint32_t i = 0; i < m_words; i++) {
            m->data[i] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t c_words = tests[curr_pos];
        c = bi_init(c_words);
        curr_pos++;

        for (uint32_t i = 0; i < c_words; i++) {
            c->data[i] = tests[curr_pos];
            curr_pos++;
        }

        pub.e = e;
        pub.n = bi_init_and_copy(n);
        priv.d = d;
        priv.n = n;

        calc_c = will_rsa_encrypt_num(m, &pub);
        calc_m = will_rsa_decrypt_num(c, &priv);

        CU_ASSERT(bi_eq(calc_c, c));
        CU_ASSERT(bi_eq(calc_m, m));

        bi_free(n);
        bi_free(e);
        bi_free(d);
        bi_free(c);
        bi_free(m);
        bi_free(calc_c);
        bi_free(calc_m);
        bi_free(pub.n);
    }
}

void test_rsa_end_to_end(void) {
    // performs a full RSA encryption/decryption test

    rsa_public_token_t pub;
    rsa_private_token_t priv;
    uint32_t seed = 1234;
    rsa_mode_t mode = RSA_MODE_512;

    gen_pub_priv_keys(seed, &pub, &priv, mode);

    MPI original_message = bi_init(1);
    bi_set(original_message, 1234);

    MPI encrypted_message = will_rsa_encrypt_num(original_message, &pub);
    MPI decrypted_message = will_rsa_decrypt_num(encrypted_message, &priv);

    // shouldn't be necessary, but leave it here for now
    bi_squeeze(decrypted_message);

    CU_ASSERT(bi_eq(original_message, decrypted_message));

    bi_free(original_message);
    bi_free(encrypted_message);
    bi_free(decrypted_message);
    bi_free(pub.e);
    bi_free(pub.n);
    bi_free(priv.d);
    bi_free(priv.n);
}

CU_pSuite register_rsa_tests(void) {
    CU_pSuite suite = CU_add_suite("RSA_Suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "rsa_keygen", test_rsa_keygen);
    CU_add_test(suite, "calc_lambda_n_d", test_calc_lambda_n_d);
    CU_add_test(suite, "rsa_end_to_end", test_rsa_end_to_end);
    CU_add_test(suite, "test_rsa_encrypt_decrypt_cases",
                test_rsa_encrypt_decrypt_cases);

    return suite;
}
