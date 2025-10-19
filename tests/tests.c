#include <stddef.h>
#include <stdio.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "test_suites.h"

int main(void) {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    register_bigint_tests();
    register_crypto_core_tests();
    register_rng_tests();
    register_rsa_tests();

    CU_basic_run_tests();

    CU_cleanup_registry();

    return 0;
}
