#ifndef TEST_SUITES_H
#define TEST_SUITES_H

#include <CUnit/CUnit.h>

CU_pSuite register_bigint_tests(void);
CU_pSuite register_bigint_signed_tests(void);
CU_pSuite register_crypto_core_tests(void);
CU_pSuite register_rng_tests(void);
CU_pSuite register_rsa_tests(void);

#endif
