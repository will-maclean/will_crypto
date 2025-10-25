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

void test_bi_gcd(void) {
    //   a_words, a[0]..,  b_words, b[0]..,  expected_words, expected[0]..
    // clang-format off
    uint32_t tests[] = {
        1, 0x00000000,   1, 0x00000000,   1, 0x00000000,
        1, 0x00000000,   1, 0x00000024,   1, 0x00000024,
        1, 0x0000000C,   1, 0x00000024,   1, 0x0000000C,
        1, 0x00000007,   1, 0x00000014,   1, 0x00000001,
        1, 0xFFFFFFFF,   1, 0x00000002,   1, 0x00000001,
        1, 0x00010000,   1, 0x00100000,   1, 0x00010000,
        1, 0x00000001,   1, 0x00000001,   1, 0x00000001,
        2, 0x00000000, 0x00000001,   1, 0x00000003,   1, 0x00000001,
        3, 0x00000000, 0x00000000, 0x00000001,   2, 0x00010000, 0x00000000,   2, 0x00010000, 0x00000000,
        3, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,   1, 0xFFFFFFFF,   1, 0xFFFFFFFF,
        2, 0x00000000, 0x00001234,   2, 0x00000000, 0x00000001,   2, 0x00000000, 0x00000001,
        2, 0x89ABCDEF, 0x01234567,   2, 0x89ABCDEF, 0x01234567,   2, 0x89ABCDEF, 0x01234567,
        1, 0x00000000,   3, 0x00000000, 0x00000000, 0x00000001,   3, 0x00000000, 0x00000000, 0x00000001,
    };
    // clang-format on

    uint32_t curr = 0;
    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr++];
        MPI a = bi_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a->data[j] = tests[curr++];

        uint32_t b_words = tests[curr++];
        MPI b = bi_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b->data[j] = tests[curr++];

        uint32_t e_words = tests[curr++];
        MPI expected = bi_init(e_words);
        for (uint32_t j = 0; j < e_words; j++)
            expected->data[j] = tests[curr++];

        MPI got = bi_gcd(a, b);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\nexpected gcd=");
            bi_print(expected);
            printf("\ncalculated   =");
            bi_print(got);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(expected);
        bi_free(got);
    }
}

void test_bi_lcm(void) {
    //   a_words, a[0]..,  b_words, b[0]..,  expected_words, expected[0]..
    // clang-format off
    uint32_t tests[] = {
        1, 0x00000000,   1, 0x00000000,   1, 0x00000000,
        1, 0x00000000,   1, 0x00000024,   1, 0x00000000,
        1, 0x0000000C,   1, 0x00000024,   1, 0x00000024,
        1, 0x00000007,   1, 0x00000014,   1, 0x0000008C,
        1, 0xFFFFFFFF,   1, 0x00000002,   2, 0xFFFFFFFE, 0x00000001,
        1, 0x00010000,   1, 0x00100000,   1, 0x00100000,
        1, 0x00000001,   1, 0x00000001,   1, 0x00000001,
        2, 0x00000000, 0x00000001,   1, 0x00000003,   2, 0x00000000, 0x00000003,
        3, 0x00000000, 0x00000000, 0x00000001,   2, 0x00010000, 0x00000000,   3, 0x00000000, 0x00000000, 0x00000001,
        3, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,   1, 0xFFFFFFFF,   3, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        2, 0x00000000, 0x00001234,   2, 0x00000000, 0x00000001,   2, 0x00000000, 0x00001234,
        2, 0x89ABCDEF, 0x01234567,   2, 0x89ABCDEF, 0x01234567,   2, 0x89ABCDEF, 0x01234567,
        1, 0x00000000,   3, 0x00000000, 0x00000000, 0x00000001,   1, 0x00000000,
    };
    // clang-format on

    uint32_t curr = 0;
    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr++];
        MPI a = bi_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a->data[j] = tests[curr++];

        uint32_t b_words = tests[curr++];
        MPI b = bi_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b->data[j] = tests[curr++];

        uint32_t e_words = tests[curr++];
        MPI expected = bi_init(e_words);
        for (uint32_t j = 0; j < e_words; j++)
            expected->data[j] = tests[curr++];

        MPI got = bi_lcm(a, b);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\nexpected lcm=");
            bi_print(expected);
            printf("\ncalculated   =");
            bi_print(got);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(expected);
        bi_free(got);
    }
}

void test_bi_mod_exp(void) {
    // clang-format off
    uint32_t tests[] = {
        // a words, ... a data, b words, ...b data, m words, ... m data, out words, ...out data
        // single word 
        1, 0, 1, 0, 1, 7, 1, 1,
        1, 2, 1, 5, 1, 0xD, 1, 6,
        1, 3, 1, 0xA, 1, 0xB, 1, 1,
        1, 0xffffffff, 1, 2, 1, 0xfffffffb, 1, 0x10,
        1, 7, 1, 5, 1, 0xF, 1, 7,
        // multi-word 
        2, 2, 0, 2, 3, 0, 2, 0xB, 0, 2, 8, 0,
        2, 0, 1, 2, 2, 0, 2, 0x13, 0, 2, 0x11, 0,
        2, 0xffffffff, 0xffffffff, 3, 0, 2, 0, 3, 0x11, 0, 0, 1, 0,
        2, 0x9ABCDEF0, 0x12345678, 3, 5, 0, 0, 2, 0x00010000, 0, 1, 0,
        2, 0x12345678, 0x87654321, 3, 2, 0, 1, 3, 0, 1, 0, 1, 0,
    };
    // clang-format on
    uint32_t curr_pos = 0;

    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos];
        MPI a = bi_init(a_words);
        curr_pos++;

        for (uint32_t j = 0; j < a_words; j++) {
            a->data[j] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t b_words = tests[curr_pos];
        MPI b = bi_init(b_words);
        curr_pos++;

        for (uint32_t j = 0; j < b_words; j++) {
            b->data[j] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t m_words = tests[curr_pos];
        MPI m = bi_init(m_words);
        curr_pos++;

        for (uint32_t j = 0; j < m_words; j++) {
            m->data[j] = tests[curr_pos];
            curr_pos++;
        }

        MPI res = bi_mod_exp(a, b, m);

        uint32_t expected_res_words = tests[curr_pos];
        MPI expected_res = bi_init(expected_res_words);
        curr_pos++;

        for (uint32_t j = 0; j < expected_res_words; j++) {
            expected_res->data[j] = tests[curr_pos];
            curr_pos++;
        }

        bool pass = bi_eq(res, expected_res);

        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\nm=");
            bi_print(m);
            printf("\ncalculated (a^b)%%m=");
            bi_print(res);
            printf("\nexpected res      =");
            bi_print(expected_res);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(m);
        bi_free(res);
        bi_free(expected_res);
    }
}

void test_bi_add(void) {
    // clang-format off
    uint32_t tests[] = {
        // a words, ...a, b words, ...b, res words, ...res
        1, 0xFFFFFFFFu, 1, 0x00000001u, 2, 0x00000000u, 0x00000001u,
        2, 0xFFFFFFFFu, 0x00000001u,   1, 0x00000001u, 2, 0x00000000u, 0x00000002u,
        3, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u, 3, 0x00000001u, 0x00000000u, 0x00000001u, 3, 0x00000000u, 0x00000000u, 0x00000002u,
        3, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 3, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 4, 0xFFFFFFFEu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000001u,
        3, 0xFFFFFFFFu, 0x00000000u, 0xFFFFFFFFu, 3, 0x00000001u, 0xFFFFFFFFu, 0x00000000u, 4, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000001u,
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

        uint32_t exp_words = tests[curr_pos++];
        MPI expected = bi_init(exp_words);
        for (uint32_t j = 0; j < exp_words; j++)
            expected->data[j] = tests[curr_pos++];

        MPI got = bi_add(a, b);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("case %d\na=", test);
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\ncalculated a+b=");
            bi_print(got);
            printf("\nexpected      =");
            bi_print(expected);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(got);
        bi_free(expected);

        test++;
    }
}

void test_bi_sub(void) {
    // clang-format off
    uint32_t tests[] = {
        // a words, ...a, b words, ...b, res words, ...res
        3, 0xFFFFFFFFu, 0x7FFFFFFFu, 0x08400000u, 1, 1, 3, 0xFFFFFFFEu, 0x7FFFFFFFu, 0x08400000u,
        3, 0, 0, 0x840, 1, 1, 3, 0xffffffff, 0xffffffff,0x83f,
        3, 0x00000000u, 0x00000000u, 0x00000001u, 1, 1, 3, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u,
        3, 0x00000000u, 0x00000002u, 0x00000001u, 2, 0x00000001u, 0x00000001u, 3, 0xFFFFFFFFu, 0x00000000u, 0x00000001u,
        4, 0x00000000u, 0x00000000u, 0x00000000u, 0x00000001u, 1, 1, 4, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0x00000000u,
        4, 0x00000000u, 0x12345678u, 0x00000001u, 0x00000001u, 1, 1, 4, 0xFFFFFFFFu, 0x12345677u, 0x00000001u, 0x00000001u,
        4, 0x00000010u, 0x00000000u, 0x00000001u, 0x00000001u, 4, 0x00000020u, 0x00000000u, 0x00000000u, 0x00000000u, 4, 0xFFFFFFF0u, 0xFFFFFFFFu, 0x00000000u, 0x00000001u,
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

        uint32_t exp_words = tests[curr_pos++];
        MPI expected = bi_init(exp_words);
        for (uint32_t j = 0; j < exp_words; j++)
            expected->data[j] = tests[curr_pos++];

        MPI got = bi_sub(a, b);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("case %d\na=", test);
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\ncalculated a-b=");
            bi_print(got);
            printf("\nexpected       =");
            bi_print(expected);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(got);
        bi_free(expected);

        test++;
    }
}

void test_bi_shift_left(void) {
    // clang-format off
    uint32_t tests[] = {
        1, 0x00000000,                  0,  1, 0x00000000,
        1, 0x89ABCDEF,                  0,  1, 0x89ABCDEF,
        1, 0x00000001,                  1,  1, 0x00000002,
        1, 0x40000000,                  1,  1, 0x80000000,
        1, 0x80000000,                  1,  2, 0x00000000, 0x00000001,
        2, 0xFFFFFFFF, 0x00000000,      4,  2, 0xFFFFFFF0, 0x0000000F,
        2, 0x89ABCDEF, 0x01234567,     32,  3, 0x00000000, 0x89ABCDEF, 0x01234567,
        3, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC,  64,  5, 0x00000000, 0x00000000, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC,
        3, 0xFFFFFFFF, 0x00000000, 0x40000000,  1,  3, 0xFFFFFFFE, 0x00000001, 0x80000000,
        3, 0x00000001, 0x00000000, 0x00000000, 36,  2, 0x00000000, 0x00000010,
        2, 0xFFFFFFFF, 0x00000001,     31,  2, 0x80000000, 0xFFFFFFFF,
        1, 0x00000001,                 32,  2, 0x00000000, 0x00000001,
        1, 0x00000000,                100,  1, 0x00000000,
        1, 0x00000001,                100,  4, 0x00000000, 0x00000000, 0x00000000, 0x00000010,
        2, 0xFFFFFFFF, 0xFFFFFFFF,      4,  3, 0xFFFFFFF0, 0xFFFFFFFF, 0x0000000F,
    };
    // clang-format on

    uint32_t curr_pos = 0;
    uint32_t test = 0;
    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos++];
        MPI a = bi_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a->data[j] = tests[curr_pos++];

        uint32_t n = tests[curr_pos++];

        uint32_t exp_words = tests[curr_pos++];
        MPI expected = bi_init(exp_words);
        for (uint32_t j = 0; j < exp_words; j++)
            expected->data[j] = tests[curr_pos++];

        MPI got = bi_shift_left(a, n);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("case %d\na=", test);
            bi_print(a);
            printf("\nn=%u", n);
            printf("\ncalculated a<<n=");
            bi_print(got);
            printf("\nexpected       =");
            bi_print(expected);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(got);
        bi_free(expected);

        test++;
    }
}

void test_bi_mod(void) {
    // clang-format off
    uint32_t tests[] = {
        // a words, ... a data, b words, ...b data, out words, ...out data
        // single word a and b
        1, 0, 1, 0xffffffff, 1, 0,
        1, 0xffffffff, 1, 1, 1, 0,
        1, 0xa, 1, 0x10, 1, 0xa,
        1, 12345678, 1, 12345678, 1, 0,
        1, 0xffffffff, 1, 0x12345678, 1, 0x0123456f,
        // multi-word a and b
        2, 0, 1, 2, 3, 0, 1, 1,
        2, 0xffffffff, 0xffffffff, 2, 0, 1, 1, 0xffffffff,
        2, 0x9ABCDEF0, 0x12345678, 2, 0x00010000, 0x00000000, 1, 0x0000DEF0,
        2, 0x10, 0x0, 2, 0x20, 0x0, 1, 0x10,
        2, 0x12345678, 0x87654321, 2, 0x12345678, 0x87654321, 1, 0x0,
    };
    // clang-format on
    uint32_t curr_pos = 0;

    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos];
        MPI a = bi_init(a_words);
        curr_pos++;

        for (uint32_t j = 0; j < a_words; j++) {
            a->data[j] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t b_words = tests[curr_pos];
        MPI b = bi_init(b_words);
        curr_pos++;

        for (uint32_t j = 0; j < b_words; j++) {
            b->data[j] = tests[curr_pos];
            curr_pos++;
        }

        MPI res;
        bi_eucl_div(a, b, NULL, &res);

        uint32_t expected_res_words = tests[curr_pos];
        MPI expected_res = bi_init(expected_res_words);
        curr_pos++;

        for (uint32_t j = 0; j < expected_res_words; j++) {
            expected_res->data[j] = tests[curr_pos];
            curr_pos++;
        }

        bool pass = bi_eq(res, expected_res);

        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\ncalculated a%%b=");
            bi_print(res);
            printf("\nexpected res   =");
            bi_print(expected_res);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(res);
        bi_free(expected_res);
    }
}
void test_bi_mul(void) {
    // clang-format off
    uint32_t tests[] = {
        // a words, ... a data, b words, ...b data, out words, ...out data
        // single word a and b
        1, 2, 1, 3, 1, 6,
        1, 0, 1, 0x12345678, 1, 0,
        1, 1, 1, 0xffffffff, 1, 0xffffffff,
        1, 0xffffffff, 1, 0xffffffff, 2, 0x1, 0xfffffffe,
        1, 0x80000000, 1, 0x2, 2, 0, 1,
        1, 0x89ABCDEF, 1, 0x01234567, 2, 0xC94E4629, 0x009CA39D,
        // multi-word a and b
        2, 0, 1, 2, 0, 1, 3, 0, 0, 1,
        2, 1, 0, 2, 2, 0, 1, 2,
        2, 0xffffffff, 0xffffffff, 2, 2, 0, 3, 0xfffffffe, 0xffffffff, 1,
        2, 0x9ABCDEF0, 0x12345678, 2, 0x87654321, 0x0FEDCBA9, 4, 0xE5618CF0, 0x2236D88F, 0xAD77D742, 0x0121FA00,
        2, 0, 0, 2, 0x12345678, 0x87654321, 1, 0,
    };
    // clang-format on
    uint32_t curr_pos = 0;
    uint32_t test = 0;

    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos];
        MPI a = bi_init(a_words);
        curr_pos++;

        for (uint32_t j = 0; j < a_words; j++) {
            a->data[j] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t b_words = tests[curr_pos];
        MPI b = bi_init(b_words);
        curr_pos++;

        for (uint32_t j = 0; j < b_words; j++) {
            b->data[j] = tests[curr_pos];
            curr_pos++;
        }

        MPI res = bi_mul(a, b);

        uint32_t expected_res_words = tests[curr_pos];
        MPI expected_res = bi_init(expected_res_words);
        curr_pos++;

        for (uint32_t j = 0; j < expected_res_words; j++) {
            expected_res->data[j] = tests[curr_pos];
            curr_pos++;
        }

        bool pass = bi_eq(res, expected_res);

        CU_ASSERT(pass);
        if (!pass) {
            printf("test case: %d\na=", test);
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\ncalculated a*b=");
            bi_print(res);
            printf("\nexpected res  =");
            bi_print(expected_res);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(res);
        bi_free(expected_res);
        test++;
    }
}
void test_bi_pow(void) {
    // single word first
    MPI a = bi_init(1);
    MPI expected_res = bi_init(1);
    bi_set(a, 3);
    bi_set(expected_res, 9);
    MPI res = bi_pow_imm(a, 2);
    CU_ASSERT(bi_eq(res, expected_res));

    bi_free(a);
    bi_free(res);
    bi_free(expected_res);

    // clang-format off
    uint32_t tests[] = {
        // a words, ... a data, b, out words, ...out data
        // single word a
        1, 3, 2, 1, 9,
        1, 0x0, 0, 1, 1,
        1, 0, 5, 1, 0,
        1, 1, 123456789, 1, 1,
        1, 2, 32, 2, 0, 1,
        1, 0xFFFFFFFF, 2, 2, 1, 0xFFFFFFFE,
        // multi-word a
        2, 0, 0, 0, 1, 1,
        2, 5, 0, 3, 1, 0x7d,
        2, 0, 1, 2, 3, 0, 0, 1,
        2, 0, 2, 5, 6, 0, 0, 0, 0, 0, 0x20,
        2, 0xffffffff, 0xffffffff, 2, 4, 1, 0, 0xfffffffe, 0xffffffff,
    };
    // clang-format on
    uint32_t n_tests = 11;
    uint32_t curr_pos = 0;

    for (uint32_t i = 0; i < n_tests; i++) {
        uint32_t a_words = tests[curr_pos];
        MPI a = bi_init(a_words);
        curr_pos++;

        for (uint32_t j = 0; j < a_words; j++) {
            a->data[j] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t b = tests[curr_pos];
        curr_pos++;

        MPI res = bi_pow_imm(a, b);

        uint32_t expected_res_words = tests[curr_pos];
        MPI expected_res = bi_init(expected_res_words);
        curr_pos++;

        for (uint32_t j = 0; j < expected_res_words; j++) {
            expected_res->data[j] = tests[curr_pos];
            curr_pos++;
        }

        bool pass = bi_eq(res, expected_res);

        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            bi_print(a);
            printf("\nb=%d\ncalculated a^b=", b);
            bi_print(res);
            printf("\nexptected res=");
            bi_print(expected_res);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(res);
        bi_free(expected_res);
    }
}
void test_bi_shift_right(void) {
    // clang-format off
    uint32_t tests[] = {
        1, 0x00000000,                 0,   1, 0x00000000,
        1, 0x89ABCDEF,                 0,   1, 0x89ABCDEF,
        1, 0x00000001,                 1,   1, 0x00000000,
        1, 0x80000000,                 1,   1, 0x40000000,
        1, 0xFFFFFFFF,                 4,   1, 0x0FFFFFFF,
        2, 0x00000000, 0x00000001,     4,   1, 0x10000000,
        2, 0x89ABCDEF, 0x01234567,    32,   1, 0x01234567,
        3, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC,  64,   1, 0xCCCCCCCC,
        3, 0xFFFFFFFF, 0x00000000, 0x80000000,  1,    3, 0x7FFFFFFF, 0x00000000, 0x40000000,
        3, 0x00000000, 0x00000000, 0x00000001, 36,    1, 0x10000000,
        2, 0xFFFFFFFF, 0xFFFFFFFF,    31,   2, 0xFFFFFFFF, 0x00000001,
        1, 0x00000001,                32,   1, 0x00000000,
        2, 0x00000001, 0x00000000,   100,   1, 0x00000000,
        4, 0x00000001, 0x00000000, 0x00000000, 0x80000000, 33,   3, 0x00000000, 0x00000000, 0x40000000,
    };
    // clang-format on

    uint32_t curr_pos = 0;
    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos++];
        MPI a = bi_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a->data[j] = tests[curr_pos++];

        uint32_t n = tests[curr_pos++];

        uint32_t exp_words = tests[curr_pos++];
        MPI expected = bi_init(exp_words);
        for (uint32_t j = 0; j < exp_words; j++)
            expected->data[j] = tests[curr_pos++];

        MPI got = bi_shift_right(a, n);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            bi_print(a);
            printf("\nn=%u", n);
            printf("\ncalculated a>>n=");
            bi_print(got);
            printf("\nexpected       =");
            bi_print(expected);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(got);
        bi_free(expected);
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

    CU_ASSERT(!bi_even(a));
    CU_ASSERT(bi_even(b));

    // comparisons
    CU_ASSERT(bi_gt(a, b));
    CU_ASSERT(!bi_gt(b, a));
    CU_ASSERT(!bi_lt(a, b));
    CU_ASSERT(bi_lt(b, a));
    CU_ASSERT(bi_ge(a, b));
    CU_ASSERT(!bi_ge(b, a));
    CU_ASSERT(!bi_le(a, b));
    CU_ASSERT(bi_le(b, a));

    bi_free(a);
    bi_free(b);
    a = bi_init(2);
    b = bi_init(2);
    a->data[1] = 1;
    a->data[0] = 5;
    b->data[1] = 1;
    b->data[0] = 2;

    CU_ASSERT(bi_gt(a, b));
    CU_ASSERT(!bi_gt(b, a));
    CU_ASSERT(!bi_lt(a, b));
    CU_ASSERT(bi_lt(b, a));
    CU_ASSERT(bi_ge(a, b));
    CU_ASSERT(!bi_ge(b, a));
    CU_ASSERT(!bi_le(a, b));
    CU_ASSERT(bi_le(b, a));

    bi_free(a);
    bi_free(b);
    a = bi_init(1);
    b = bi_init(1);
    bi_set(a, 5u);
    bi_set(b, 2u);

    // addition
    bi_set(expected_res, 7u);
    res = bi_add(a, b);
    CU_ASSERT(bi_eq(res, expected_res));
    bi_free(res);

    // subtraction
    bi_set(expected_res, 3u);
    res = bi_sub(a, b);
    CU_ASSERT(bi_eq(res, expected_res));
    bi_free(res);

    // multiplication
    bi_set(expected_res, 10u);
    res = bi_mul(a, b);
    bool passed = bi_eq(res, expected_res);
    CU_ASSERT(passed);
    if (!passed) {
        printf("a, b, res:\n");
        bi_print(a);
        printf("\n");
        bi_print(b);
        printf("\n");
        bi_print(res);
        printf("\n\n");
    }
    bi_free(res);

    // integer division
    bi_set(expected_res, 2u);
    bi_eucl_div(a, b, &res, NULL);
    CU_ASSERT(bi_eq(res, expected_res));
    bi_free(res);

    // integer division, multi-word
    MPI a_euc = bi_init(2);
    MPI b_euc = bi_init(1);
    a_euc->data[1] = 1;
    a_euc->data[0] = 2;
    b_euc->data[0] = 2;
    bi_set(expected_res, 0x80000001);
    bi_eucl_div(a_euc, b_euc, &res, NULL);
    passed = bi_eq(res, expected_res);

    CU_ASSERT(passed);
    if (!passed) {
        printf("a, b, res:\n");
        bi_print(a_euc);
        printf("\n");
        bi_print(b_euc);
        printf("\n");
        bi_print(res);
        printf("\n\n");
    }
    bi_free(res);
    bi_free(b_euc);
    bi_free(a_euc);

    // modulo
    bi_set(expected_res, 1u);
    bi_eucl_div(a, b, NULL, &res);
    CU_ASSERT(bi_eq(res, expected_res));
    bi_free(res);

    // inc
    bi_set(expected_res, 6u);
    bi_set(a, 5u);
    bi_inc(a);
    CU_ASSERT(bi_eq(a, expected_res));
    bi_set(a, 5u);

    // dec
    bi_set(expected_res, 4u);
    bi_set(a, 5u);
    bi_dec(a);
    CU_ASSERT(bi_eq(a, expected_res));
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
    CU_ASSERT(passed);
    if (!passed) {
        printf("a, expected_res:\n");
        bi_print(a);
        printf("\n");
        bi_print(expected_res);
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
    CU_ASSERT(bi_eq(b, expected_res));
    bi_set(a, 5u);
    bi_set(b, 3u);

    bi_free(a);
    bi_free(b);
    a = bi_init(2);
    a->data[0] = (1ull << 31);
    bi_free(expected_res);
    expected_res = bi_init(2);
    expected_res->data[1] = 1u;
    b = bi_shift_left(a, 1);
    CU_ASSERT(bi_eq(b, expected_res));
    bi_free(a);
    a = bi_init(1);
    bi_set(a, 5u);

    // shift right
    bi_free(b);
    bi_set(a, 5u);
    bi_set(expected_res, 2u);
    b = bi_shift_right(a, 1u);
    CU_ASSERT(bi_eq(b, expected_res));
    bi_set(a, 5u);
    bi_set(b, 3u);

    bi_free(a);
    a = bi_init(2);
    a->data[1] = 1u;
    bi_free(expected_res);
    expected_res = bi_init(1);
    expected_res->data[0] = (1ull << 31);
    bi_free(b);
    b = bi_shift_right(a, 1);
    CU_ASSERT(bi_eq(b, expected_res));
    bi_free(a);
    a = bi_init(1);
    bi_set(a, 5u);

    bi_free(a);
    bi_free(b);
    bi_free(expected_res);
}

void test_bigint(void) {
    // test create and free
    int test_words = 2;

    MPI x, y, z;

    x = bi_init(4);
    bi_squeeze(x);
    CU_ASSERT(x->words == 1);
    bi_free(x);

    x = bi_init(4);
    x->data[1] = 42u;
    bi_squeeze(x);
    CU_ASSERT(x->words == 2);
    bi_free(x);

    // test init and free
    x = bi_init(test_words);
    bi_free(x);

    // test init_like
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

    z = bi_add(x, y);
    bi_free(z);

    z = bi_sub(y, x);
    bi_free(z);

    for (int i = 0; i < 3; i++) {
        z = bi_mul(x, y);
        bi_copy(z, y);
        bi_free(z);
    }

    // only least sig fig example
    bi_set(x, 5u);
    bi_set(y, 3u);
    bi_eucl_div(x, y, &z, NULL);

    bi_free(x);
    bi_free(y);
    bi_free(z);
}

void test_ext_euc(void) {
    //   a_words, a data..., b_words, b data..., gcd_words, gcd data...
    // clang-format off
    uint32_t tests[] = {
        1, 0x0000000C, 1, 0x00000024, 1, 0x0000000C,
        1, 0x0000001E, 1, 0x00000015, 1, 0x00000003,
        1, 0x000000F0, 1, 0x0000002E, 1, 0x00000002,
        1, 0x00000187, 1, 0x0000012B, 1, 0x00000017,
        1, 0x00000011, 1, 0x00000138, 1, 0x00000001,
        1, 0x00000003, 1, 0x00000002, 1, 0x00000001,
        1, 0x00000000, 1, 0x00000005, 1, 0x00000005,
        2, 0x00000000, 0x00000003, 2, 0x00000000, 0x00000002, 2, 0x00000000, 0x00000001,
        2, 0x12345678, 0x00000005, 2, 0x00001000, 0x00000003, 1, 0x00000008,
        3, 0x00001234, 0x00000000, 0x00000001, 2, 0x00005678, 0x00000001, 1, 0x00000004,
        3, 0x00000000, 0x00000000, 0x00000005, 3, 0x00000000, 0x00000000, 0x00000002, 3, 0x00000000, 0x00000000, 0x00000001,
        1, 0x00000015, 3, 0x000000FF, 0x00000000, 0x00000001, 1, 0x00000001,
        2, 0x00008000, 0x00000004, 2, 0x00004000, 0x00000002, 2, 0x00004000, 0x00000002,
        3, 0x34567890, 0x00001234, 0x00000002, 2, 0x0000FEDC, 0x00000001, 1, 0x00000004,
        2, 0x00008000, 0x00000001, 1, 0x00020000, 1, 0x00008000,
        3, 0x00000005, 0x00000000, 0x00000005, 3, 0x00000007, 0x00000000, 0x00000007, 3, 0x00000001, 0x00000000, 0x00000001,
        2, 0x00000010, 0x00000001, 2, 0x00000020, 0x00000001, 1, 0x00000010,
    };
    // clang-format on

    size_t curr = 0;
    size_t case_idx = 1;
    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr++];
        MPI a = bi_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a->data[j] = tests[curr++];

        uint32_t b_words = tests[curr++];
        MPI b = bi_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b->data[j] = tests[curr++];

        uint32_t g_words = tests[curr++];
        MPI expected_gcd = bi_init(g_words);
        for (uint32_t j = 0; j < g_words; j++)
            expected_gcd->data[j] = tests[curr++];

        ext_euc_res_t res = ext_euc(a, b);

        bool gcd_sign_ok = res.gcd.positive;
        bool gcd_val_ok = bi_eq(res.gcd.val, expected_gcd);
        if (!gcd_sign_ok || !gcd_val_ok) {
            printf("ext_euc gcd failure (case %zu)\n", case_idx);
            printf("a=");
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\nexpected gcd=");
            bi_print(expected_gcd);
            printf("\nactual gcd (sign=%d)=", res.gcd.positive ? 1 : 0);
            bi_print(res.gcd.val);
            printf("\n\n");
        }
        CU_ASSERT(gcd_sign_ok);
        CU_ASSERT(gcd_val_ok);

        sMPI signed_a = from_unsigned(a, true);
        sMPI signed_b = from_unsigned(b, true);
        sMPI term1 = signed_mul(res.bez_x, signed_a);
        sMPI term2 = signed_mul(res.bez_y, signed_b);
        sMPI combo = signed_add(term1, term2);

        bool identity_mag_ok = bi_eq(combo.val, res.gcd.val);
        bool identity_sign_ok =
            combo.positive == res.gcd.positive || bi_eq_val(combo.val, 0);
        bool identity_ok = identity_mag_ok && identity_sign_ok;
        if (!identity_ok) {
            printf("ext_euc identity failure (case %zu)\n", case_idx);
            printf("a=");
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\nbez_x (sign=%d)=", res.bez_x.positive ? 1 : 0);
            bi_print(res.bez_x.val);
            printf("\nbez_y (sign=%d)=", res.bez_y.positive ? 1 : 0);
            bi_print(res.bez_y.val);
            printf("\na*x + b*y (sign=%d)=", combo.positive ? 1 : 0);
            bi_print(combo.val);
            printf("\ncomputed gcd (sign=%d)=", res.gcd.positive ? 1 : 0);
            bi_print(res.gcd.val);
            printf("\nexpected gcd=");
            bi_print(expected_gcd);
            printf("\n\n");
        }
        CU_ASSERT(identity_ok);

        signed_free(combo);
        signed_free(term1);
        signed_free(term2);
        signed_free(signed_a);
        signed_free(signed_b);

        bi_free(a);
        bi_free(b);
        bi_free(expected_gcd);
        signed_free(res.bez_x);
        signed_free(res.bez_y);
        signed_free(res.gcd);

        case_idx++;
    }
}

void test_bi_mul_inv_mod(void) {
    // clang-format off
    uint32_t tests[] = {
        // a words, ...a, b words, ...b, res words, ...res
        3, 1, 0, 1, 3, 1, 0, 2, 1, 2,
        3, 3, 0, 1, 3, 1, 1, 1, 2, 0x4924924a, 0x6db6db6e,
        3, 5, 2, 1, 3, 1, 3, 1, 2, 0xe58469ef, 0x69ee5847,
        3, 7, 4, 1, 3, 1, 5, 1, 2, 0x2dd9ca83, 0xece540f9,
        3, 9, 6, 1, 3, 1, 7, 1, 2, 0xa50658dd, 0xd7cd3925,
        1, 0x00000002, 1, 0x00000003, 1, 0x00000002,
        1, 0x00000003, 1, 0x00000007, 1, 0x00000005,
        1, 0x0000000A, 1, 0x00000011, 1, 0x0000000C,
        1, 0x00000005, 1, 0x00000011, 1, 0x00000007,
        1, 0x00000007, 1, 0x00000019, 1, 0x00000012,
        1, 0x0000000F, 1, 0x00000025, 1, 0x00000005,
        1, 0x00000013, 1, 0x0000001F, 1, 0x00000012,
        1, 0x0000001D, 1, 0x00000023, 1, 0x0000001D,
        1, 0x00000025, 1, 0x0000002B, 1, 0x00000007,
        1, 0x0000002F, 1, 0x00000035, 1, 0x0000002C,
        1, 0x00000033, 1, 0x00000037, 1, 0x00000029,
        1, 0x00000039, 1, 0x0000003D, 1, 0x0000000F,
        1, 0x0000003B, 1, 0x00000041, 1, 0x00000036,
        2, 0x00000003, 0x00000001, 2, 0x00000005, 0x00000002, 1, 0x00000002,
        3, 0xDEF12345, 0x56789ABC, 0x00001234, 2, 0x00000001, 0xFFFFFFFF, 2, 0xF0533D88, 0xB799F7B7,
        3, 0x87654321, 0xFFEDCBA9, 0x000ABCDE, 3, 0x00000001, 0x00000000, 0x10000000, 3, 0x769B039A, 0x8BC9156F, 0x01918F6C,
        2, 0x41381F41, 0x0000A23C, 2, 0xB436AD0D, 0x41BC14B1, 2, 0xEFC2D419, 0x09899F4F,
        2, 0xD14AB787, 0x0000DB51, 2, 0x53CD374B, 0x6201711F, 2, 0x1FD90047, 0x07C74CA3,
        3, 0x2EE4F703, 0x2034E3E7, 0x000C368C, 3, 0x7AAD4CDD, 0x51AA0379, 0x527B9776, 3, 0x4E0C4EAB, 0x42289309, 0x0DFAB1E8,
        3, 0xAA2855A3, 0x2BD30C4E, 0x000DE7D6, 3, 0xE6FF5DE1, 0x0D4DB57B, 0x7AD86C9C, 3, 0xCAFF3B35, 0xA50E68D5, 0x4591D820,
        4, 0x742BA403, 0x3074FFD2, 0xA4113D22, 0x00C7E14E, 4, 0x8B030A2F, 0xB571719A, 0xA214E4AC, 0x51C5FD8F, 4, 0x8977040A, 0x69FAACB3, 0xD57B4E03, 0x4C006477,
        4, 0x584FD91D, 0x3A03B568, 0x901182CD, 0x00F9B035, 4, 0xB6D54149, 0x66C33328, 0xD5207BF3, 0x68D6ECBE, 4, 0x830A0F1A, 0xD3AD334F, 0x80042E1A, 0x61A611A3,
        5, 0x0B425133, 0xD278F089, 0xBAA8BFC8, 0xF6E9A553, 0x0D0C8A2D, 5, 0x9B90AA5B, 0x8A920826, 0xFE33C6E3, 0xCA5710E3, 0x724AB806, 5, 0x1507AC70, 0xBE99D82D, 0x0B661C25, 0x8AE41B45, 0x396A507F,
        3, 0x5410BF27, 0x8B72DF4F, 0x000DFB7E, 3, 0x1D0D8901, 0x33932B0F, 0x6141FA90, 3, 0x586D381B, 0x4EECAB1C, 0x28F07D31,
        4, 0x31FBBDF9, 0x9A52AA09, 0x7CFC127B, 0x00E6DEAE, 4, 0xBC0CF3C3, 0xE32216B2, 0x4DF33F69, 0x611EA05F, 4, 0xDE80A8C5, 0xE0802320, 0x9D58C957, 0x0DCAB087,
        5, 0x022FB0E5, 0xE68D3938, 0xB31B767F, 0x89CFBEAF, 0x0F786CDD, 5, 0xCFCA97AD, 0x7EE038F3, 0x91D924B5, 0x52AF0C40, 0x5D819DFD, 5, 0xB86C27E3, 0x7BAD8E0A, 0x1B32D551, 0xE153731E, 0x22736774,
    };
    // clang-format on

    uint32_t curr_pos = 0;
    uint32_t test = 0;

    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr_pos];
        MPI a = bi_init(a_words);
        curr_pos++;

        for (uint32_t j = 0; j < a_words; j++) {
            a->data[j] = tests[curr_pos];
            curr_pos++;
        }

        uint32_t b_words = tests[curr_pos];
        MPI b = bi_init(b_words);
        curr_pos++;

        for (uint32_t j = 0; j < b_words; j++) {
            b->data[j] = tests[curr_pos];
            curr_pos++;
        }

        MPI res = bi_mod_mult_inv(a, b);

        uint32_t expected_res_words = tests[curr_pos];
        MPI expected_res = bi_init(expected_res_words);
        curr_pos++;

        for (uint32_t j = 0; j < expected_res_words; j++) {
            expected_res->data[j] = tests[curr_pos];
            curr_pos++;
        }

        bool pass = bi_eq(res, expected_res);

        CU_ASSERT(pass);
        if (!pass) {
            printf("test case: %d\na=", test);
            bi_print(a);
            printf("\nb=");
            bi_print(b);
            printf("\ncalculated a^-1 mod b=");
            bi_print(res);
            printf("\nexpected res         =");
            bi_print(expected_res);
            printf("\n\n");
        }

        bi_free(a);
        bi_free(b);
        bi_free(res);
        bi_free(expected_res);
        test++;
    }
}

CU_pSuite register_bigint_tests(void) {
    CU_pSuite suite = CU_add_suite("BIGINT_Suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "bi_add", test_bi_add);
    CU_add_test(suite, "bi_sub", test_bi_sub);
    CU_add_test(suite, "bi_shift_right", test_bi_shift_right);
    CU_add_test(suite, "bi_shift_left", test_bi_shift_left);
    CU_add_test(suite, "bi_mul", test_bi_mul);
    CU_add_test(suite, "bi_pow", test_bi_pow);
    CU_add_test(suite, "bi_mod", test_bi_mod);
    CU_add_test(suite, "bi_mod_exp", test_bi_mod_exp);
    CU_add_test(suite, "bi_gcd", test_bi_gcd);
    CU_add_test(suite, "bi_lcm", test_bi_lcm);
    CU_add_test(suite, "ext_euc", test_ext_euc);
    CU_add_test(suite, "test_bi_mul_inv_mod", test_bi_mul_inv_mod);

    return suite;
}
