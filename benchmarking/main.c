#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <rng/rng.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const uint32_t SEED = 1234u;

typedef enum { basic_a_b, void_uint32 } fn_type_t;

typedef MPI (*basic_fn)(MPI a, MPI b);
typedef MPI (*void_uint32_fn)(uint32_t);

typedef struct {
    char trial_name[64];
    fn_type_t fn_type;
    union {
        basic_fn _basic_fn;
        void_uint32_fn _void_uint32_fn;
    } fn;
    uint32_t n_trials;
    uint32_t n_words;
} test_config_t;

typedef struct {
    test_config_t *config;
    float avg_exe_ms;
} fn_benchmark_res_t;

fn_benchmark_res_t bench_basic(test_config_t *config) {
    will_rng_init(SEED);

    fn_benchmark_res_t res;
    res.config = config;

    clock_t start;
    double mean_exe_time = 0;

    // Define some MPIs for the inner loops to use
    MPI a, b, c;
    volatile MPI r;
    for (uint32_t i = 0; i < config->n_trials; i++) {
        switch (config->fn_type) {
        case basic_a_b:

            a = will_rng_next(config->n_words);
            b = will_rng_next(config->n_words);

            start = clock();
            r = config->fn._basic_fn(a, b);
            mean_exe_time += (float)(clock() - start) / (float)CLOCKS_PER_SEC;

            bi_free(a);
            bi_free(b);
            bi_free(r);
            break;
        case void_uint32:
            start = clock();
            r = config->fn._void_uint32_fn(config->n_words);
            mean_exe_time += (float)(clock() - start) / (float)CLOCKS_PER_SEC;

            bi_free(r);
            break;
        }
    }

    mean_exe_time /= config->n_trials;
    res.avg_exe_ms = 1000 * mean_exe_time;

    return res;
}

void print_results(fn_benchmark_res_t *results, uint32_t n) {
    printf("----BENCHMARK RESULTS----\n\n");

    for (uint32_t i = 0; i < n; i++) {
        fn_benchmark_res_t res = results[i];
        printf("\tFunction: %s\n", res.config->trial_name);
        printf("\tN trials: %d\n", res.config->n_trials);
        printf("\tN words for inputs: %d\n", res.config->n_words);
        printf("\tAvg exe ms: %f\n", res.avg_exe_ms);
        printf("\n");
    }

    printf("\n----END BENCHMARK RESULTS----\n");
}

int main(void) {
    test_config_t configs[] = {
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 1},
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 2},
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 4},
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 8},
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 32},
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 64},
        {"bi_add", basic_a_b, .fn._basic_fn = bi_add, 100, 128},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 1},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 2},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 4},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 8},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 32},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 64},
        {"bi_sub", basic_a_b, .fn._basic_fn = bi_sub, 100, 128},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 1},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 2},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 4},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 8},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 32},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 64},
        {"bi_mul", basic_a_b, .fn._basic_fn = bi_mul, 100, 128},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 1},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 2},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 4},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 8},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 32},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 64},
        // {"bi_eucl_div", basic_a_b, .fn._basic_fn = bi_eucl_div, 100, 128},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 1},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 2},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 4},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 8},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 32},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 64},
        // {"bi_mod", basic_a_b, .fn._basic_fn = bi_mod, 100, 128},
        {"rng", void_uint32, .fn._void_uint32_fn = will_rng_next, 100, 32},
        {"rng", void_uint32, .fn._void_uint32_fn = will_rng_next, 100, 64},
        {"rng", void_uint32, .fn._void_uint32_fn = will_rng_next, 100, 128},
        {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 100, 1},
        {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 100, 2},
        {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 100, 4},
        {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 100, 8},
        {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 10, 16},
        {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 1, 32},
        // {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 10, 64},
        // {"gen_prime", void_uint32, .fn._void_uint32_fn = gen_prime, 1, 128},
    };

    int n_configs = sizeof(configs) / sizeof(test_config_t);

    fn_benchmark_res_t *results =
        malloc(n_configs * sizeof(fn_benchmark_res_t));

    for (int j = 0; j < n_configs; j++) {
        results[j] = bench_basic(&configs[j]);
    }

    print_results(results, n_configs);
}
