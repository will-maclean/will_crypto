#include <stdint.h>

#include <bigint/bigint.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "test_suites.h"
void test_signed_sub(void) {
    //   a_words, a[0]..,  a pos, b_words, b[0]..,  b pos expected_words,
    //   expected[0].., expected pos
    // clang-format off
    uint32_t tests[] = {
        1, 0x00000001, 1, 1, 0x00000001, 1, 1, 0x00000000, 1,
        1, 0x00000005, 1, 1, 0x00000003, 1, 1, 0x00000002, 1,
        1, 0x00000003, 1, 1, 0x00000005, 1, 1, 0x00000002, 0,
        1, 0x00000003, 0, 1, 0x00000005, 0, 1, 0x00000002, 1,
        1, 0x00000005, 0, 1, 0x00000003, 1, 1, 0x00000008, 0,
        1, 0x00000008, 1, 1, 0x00000002, 0, 1, 0x0000000A, 1,
        2, 0x00000000, 0x00000002, 1, 1, 0x00000001, 1, 2, 0xFFFFFFFF, 0x00000001, 1,
        2, 0xFFFFFFFF, 0x00000001, 1, 2, 0x00000005, 0x00000001, 1, 1, 0xFFFFFFFA, 1,
        2, 0x00000000, 0x00000001, 1, 2, 0x00000001, 0x00000001, 1, 1, 0x00000001, 0,
        3, 0x00000003, 0x00000000, 0x00000001, 0, 1, 0x00000005, 0, 2, 0xFFFFFFFE, 0xFFFFFFFF, 0,
        3, 0x00000001, 0x00000002, 0x00000000, 1, 3, 0x00000001, 0x00000000, 0x00000001, 0, 3, 0x00000002, 0x00000002, 0x00000001, 1,
    };
    // clang-format on

    uint32_t curr = 0;
    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr++];
        sMPI a = signed_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a.val->data[j] = tests[curr++];
        a.val->words = a_words;
        a.positive = tests[curr++];

        uint32_t b_words = tests[curr++];
        sMPI b = signed_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b.val->data[j] = tests[curr++];
        b.val->words = b_words;
        b.positive = tests[curr++];

        uint32_t e_words = tests[curr++];
        sMPI expected = signed_init(e_words);
        for (uint32_t j = 0; j < e_words; j++)
            expected.val->data[j] = tests[curr++];
        expected.val->words = e_words;
        expected.positive = tests[curr++];

        sMPI got = signed_sub(a, b);

        bool pass = signed_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            signed_print(a);
            printf("\nb=");
            signed_print(b);
            printf("\nexpected gcd=");
            signed_print(expected);
            printf("\ncalculated   =");
            signed_print(got);
            printf("\n\n");
        }

        signed_free(a);
        signed_free(b);
        signed_free(expected);
        signed_free(got);
    }
}

void test_signed_add(void) {
    //   a_words, a[0]..,  a pos, b_words, b[0]..,  b pos expected_words,
    //   expected[0].., expected pos
    // clang-format off
    uint32_t tests[] = {
        1, 1, 1, 1, 1, 1, 1, 2, 1,
    };
    // clang-format on

    uint32_t curr = 0;
    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr++];
        sMPI a = signed_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a.val->data[j] = tests[curr++];
        a.positive = tests[curr++];

        uint32_t b_words = tests[curr++];
        sMPI b = signed_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b.val->data[j] = tests[curr++];
        b.positive = tests[curr++];

        uint32_t e_words = tests[curr++];
        sMPI expected = signed_init(e_words);
        for (uint32_t j = 0; j < e_words; j++)
            expected.val->data[j] = tests[curr++];
        expected.positive = tests[curr++];

        sMPI got = signed_add(a, b);

        bool pass = signed_eq(got, expected);
        CU_ASSERT(pass);
        if (!pass) {
            printf("a=");
            signed_print(a);
            printf("\nb=");
            signed_print(b);
            printf("\nexpected gcd=");
            signed_print(expected);
            printf("\ncalculated   =");
            signed_print(got);
            printf("\n\n");
        }

        signed_free(a);
        signed_free(b);
        signed_free(expected);
        signed_free(got);
    }
}

void test_signed_eucl_div(void) {
    //   a_words, a data..., a positive,
    //   b_words, b data..., b positive,
    //   expected_q_words, expected_q data..., expected_q positive,
    //   expected_r_words, expected_r data...
    // clang-format off
    uint32_t tests[] = {
        // a = 7, b = 3, q = 2, r = 1
        1, 7, 1, 1, 3, 1, 1, 2, 1, 1, 1,
        // a = 7, b = -3, q = -2, r = 1
        1, 7, 1, 1, 3, 0, 1, 2, 0, 1, 1,
        // a = -7, b = 3, q = -3, r = 2
        1, 7, 0, 1, 3, 1, 1, 3, 0, 1, 2,
        // a = -7, b = -3, q = 3, r = 2
        1, 7, 0, 1, 3, 0, 1, 3, 1, 1, 2,
        // a = -7, b = 3, q = -3, r = 2
        1, 0x00000007, 0, 1, 0x00000003, 1, 1, 0x00000003, 0, 1, 0x00000002,
        // a = -9, b = 3, q = -3, r = 0
        1, 0x00000009, 0, 1, 0x00000003, 1, 1, 0x00000003, 0, 1, 0x00000000,
        // a = 6, b = -3, q = -2, r = 0
        1, 0x00000006, 1, 1, 0x00000003, 0, 1, 0x00000002, 0, 1, 0x00000000,
        // a = 2, b = -5, q = 0, r = 2
        1, 0x00000002, 1, 1, 0x00000005, 0, 1, 0x00000000, 1, 1, 0x00000002,
        // a = 8589934592, b = 8, q = 1073741824, r = 0
        2, 0x00000000, 0x00000002, 1, 1, 0x00000008, 1, 1, 0x40000000, 1, 1, 0x00000000,
        // a = -36893488147419103248, b = 4294967301, q = -8589934583, r = 4294967235
        3, 0x00000010, 0x00000000, 0x00000002, 0, 2, 0x00000005, 0x00000001, 1, 2, 0xFFFFFFF7, 0x00000001, 0, 1, 0xFFFFFFC3,
        // a = 55340232223438392815, b = 18446744073709555712, q = 3, r = 2309725679
        3, 0x89ABCDEF, 0x00000000, 0x00000003, 1, 3, 0x00001000, 0x00000000, 0x00000001, 1, 1, 0x00000003, 1, 1, 0x89AB9DEF,
        // a = 18446744073709551615, b = 4600387192, q = 4009824239, r = 2442804727
        2, 0xFFFFFFFF, 0xFFFFFFFF, 1, 2, 0x12345678, 0x00000001, 1, 1, 0xEF010FEF, 1, 1, 0x919A3DF7,
        // a = -92233720368547758080, b = -2, q = 46116860184273879040, r = 0
        3, 0x00000000, 0x00000000, 0x00000005, 0, 1, 0x00000002, 0, 3, 0x00000000, 0x80000000, 0x00000002, 1, 1, 0x00000000,
        // a = 4294967297, b = -32768, q = -131072, r = 1
        2, 0x00000001, 0x00000001, 1, 1, 0x00008000, 0, 1, 0x00020000, 0, 1, 0x00000001,
        // a = -18446744073709584384, b = 32, q = -576460752303424512, r = 0
        3, 0x00008000, 0x00000000, 0x00000001, 0, 1, 0x00000020, 1, 2, 0x00000400, 0x08000000, 0, 1, 0x00000000,
        // a = 18446744073709551616, b = 4294967296, q = 4294967296, r = 0
        3, 0x00000000, 0x00000000, 0x00000001, 1, 2, 0x00000000, 0x00000001, 1, 2, 0x00000000, 0x00000001, 1, 1, 0x00000000,
        // a = 18446744073709551618, b = -4294967296, q = -4294967296, r = 2
        3, 0x00000002, 0x00000000, 0x00000001, 1, 2, 0x00000000, 0x00000001, 0, 2, 0x00000000, 0x00000001, 0, 1, 0x00000002,
        // a = -36893488147419103233, b = -4096, q = 9007199254740993, r = 4095
        4, 0x00000001, 0x00000000, 0x00000002, 0x00000000, 0, 2, 0x00001000, 0x00000000, 0, 2, 0x00000001, 0x00200000, 1, 1, 0x00000FFF,
    };
    // clang-format on


    size_t curr = 0;
    uint32_t test = 0;
    while (curr < sizeof(tests) / sizeof(uint32_t)) {
        uint32_t a_words = tests[curr++];
        sMPI a = signed_init(a_words);
        for (uint32_t j = 0; j < a_words; j++)
            a.val->data[j] = tests[curr++];
        a.val->words = a_words;
        a.positive = tests[curr++] != 0u;

        uint32_t b_words = tests[curr++];
        sMPI b = signed_init(b_words);
        for (uint32_t j = 0; j < b_words; j++)
            b.val->data[j] = tests[curr++];
        b.val->words = b_words;
        b.positive = tests[curr++] != 0u;

        uint32_t q_words = tests[curr++];
        MPI expected_q = bi_init(q_words);
        for (uint32_t j = 0; j < q_words; j++)
            expected_q->data[j] = tests[curr++];
        expected_q->words = q_words;
        bool expected_q_positive = tests[curr++] != 0u;

        uint32_t r_words = tests[curr++];
        MPI expected_remainder = bi_init(r_words);
        for (uint32_t j = 0; j < r_words; j++)
            expected_remainder->data[j] = tests[curr++];
        expected_remainder->words = r_words;

        sMPI q;
        MPI remainder = NULL;

        signed_eucl_div(a, b, &q, &remainder);

        CU_ASSERT(q.positive == expected_q_positive);
        CU_ASSERT(bi_eq(q.val, expected_q));
        CU_ASSERT_PTR_NOT_NULL(remainder);
        bool pass = (q.positive == expected_q_positive) &&
                    bi_eq(q.val, expected_q) &&
                    bi_eq(remainder, expected_remainder);
        if (remainder != NULL) {
            CU_ASSERT(bi_eq(remainder, expected_remainder));
            bi_free(remainder);
        }


        if (!pass){
            printf("\nsigned_eucl_div: failed test %d\n\n",test);
        }

        signed_free(q);
        signed_free(a);
        signed_free(b);
        bi_free(expected_q);
        bi_free(expected_remainder);

        test++;
    }

    sMPI a_only = make_small_signed(7u, true);
    sMPI b_only = make_small_signed(3u, false);
    sMPI q_only;

    signed_eucl_div(a_only, b_only, &q_only, NULL);

    CU_ASSERT(!q_only.positive);
    CU_ASSERT(bi_eq_val(q_only.val, 2u));

    signed_free(q_only);
    signed_free(a_only);
    signed_free(b_only);
}

CU_pSuite register_bigint_signed_tests(void) {
    CU_pSuite suite = CU_add_suite("BIGINT_signed_suite", NULL, NULL);

    if (!suite)
        return NULL;

    CU_add_test(suite, "test_signed_add", test_signed_add);
    CU_add_test(suite, "test_signed_sub", test_signed_sub);
    CU_add_test(suite, "signed_eucl_div", test_signed_eucl_div);

    return suite;
}
