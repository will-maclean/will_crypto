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

void test_bi_knuth_d(void) {
    // clang-format off
    uint32_t tests[] = {
        // u_words, u..., v_words, v..., return_quotient(0/1), expected_words, expected...
        1, 0x00000005, 2, 0x0000000A, 0x00000000, 1, 1, 0x00000000,
        1, 0x00000005, 2, 0x0000000A, 0x00000000, 0, 1, 0x00000005,
        2, 0x00000000, 0x00000001, 2, 0x00000000, 0x00000001, 1, 1, 0x00000001,
        2, 0x00000000, 0x00000001, 2, 0x00000000, 0x00000001, 0, 1, 0x00000000,
        2, 0x9ABCDEF0, 0x12345678, 2, 0x00010000, 0x00000000, 1, 2, 0x56789ABC, 0x00001234,
        2, 0x9ABCDEF0, 0x12345678, 2, 0x00010000, 0x00000000, 0, 1, 0x0000DEF0,
        3, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 2, 0xFFFFFFFE, 0x00000001, 1, 2, 0x00000001, 0x00000001,
        3, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 2, 0xFFFFFFFE, 0x00000001, 0, 1, 0x00000001,
        3, 0x00000000, 0x00000000, 0x00000001, 2, 0x00000001, 0x00000001, 1, 1, 0xFFFFFFFF,
        3, 0x00000000, 0x00000000, 0x00000001, 2, 0x00000001, 0x00000001, 0, 1, 0x00000001,
        3, 0x68AC5678, 0x79ACF124, 0x00009ABD, 2, 0x12345678, 0x9ABCDEF0, 1, 1, 0x00010001,
        3, 0x68AC5678, 0x79ACF124, 0x00009ABD, 2, 0x12345678, 0x9ABCDEF0, 0, 1, 0x00000000,
        3, 0xFFFFFFFF, 0x00000000, 0x00000001, 2, 0x7FFFFFFF, 0x00000002, 1, 1, 0x66666666,
        3, 0xFFFFFFFF, 0x00000000, 0x00000001, 2, 0x7FFFFFFF, 0x00000002, 0, 2, 0x66666665, 0x00000002,
    };
    // clang-format on

    uint32_t curr_pos = 0;
    uint32_t test = 0;
    while (curr_pos < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t u_words = tests[curr_pos++];
        MPI u = bi_init(u_words);
        for (uint32_t j = 0; j < u_words; j++) {
            u->data[j] = tests[curr_pos++];
        }

        uint32_t v_words = tests[curr_pos++];
        MPI v = bi_init(v_words);
        for (uint32_t j = 0; j < v_words; j++) {
            v->data[j] = tests[curr_pos++];
        }

        bool return_quotient = tests[curr_pos++] != 0;

        uint32_t exp_words = tests[curr_pos++];
        MPI expected = bi_init(exp_words);
        for (uint32_t j = 0; j < exp_words; j++)
            expected->data[j] = tests[curr_pos++];

        MPI got = knuth_d(u, v, return_quotient);

        bool pass = bi_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("test case %d\nu=", test);
            bi_print(u);
            printf("\nv=");
            bi_print(v);
            printf("\nreturn_quotient=%d", (int)return_quotient);
            printf("\ncalculated=");
            bi_print(got);
            printf("\nexpected  =");
            bi_print(expected);
            printf("\n\n");
        }

        bi_free(u);
        bi_free(v);
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

        MPI res = bi_mod(a, b);

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
    res = bi_eucl_div(a, b);
    CU_ASSERT(bi_eq(res, expected_res));
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
    res = bi_mod(a, b);
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
    z = bi_eucl_div(x, y);

    bi_free(x);
    bi_free(y);
    bi_free(z);
}

CU_pSuite register_bigint_tests(void) {
    CU_pSuite suite = CU_add_suite("BIGINT_Suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "bi_shift_right", test_bi_shift_right);
    CU_add_test(suite, "bi_shift_left", test_bi_shift_left);
    CU_add_test(suite, "bi_mul", test_bi_mul);
    CU_add_test(suite, "bi_pow", test_bi_pow);
    CU_add_test(suite, "bi_mod", test_bi_mod);
    CU_add_test(suite, "bi_mod_exp", test_bi_mod_exp);
    CU_add_test(suite, "knuth_d", test_bi_knuth_d);
    CU_add_test(suite, "bi_gcd", test_bi_gcd);
    CU_add_test(suite, "bi_lcm", test_bi_lcm);

    return suite;
}
