#include "rsa.h"
#include "bigint.h"
#include "primality.h"
#include "rng.h"
#include <stdio.h>

void load_new_primes(struct rsa_state *new_state, uint32_t seed,
                     rsa_mode_t mode) {
    uint32_t prime_words;
    switch (mode) {
    case RSA_MODE_1024:
        prime_words = 16;
        break;
    case RSA_MODE_512:
        prime_words = 8;
        break;
    }

    struct will_rng_cfg rng_cfg;
    rng_cfg.words = prime_words;

    init_will_rng(&rng_cfg, seed);

    new_state->p = gen_prime(prime_words);
    new_state->q = gen_prime(prime_words);
}

typedef struct {
    MPI val;
    bool positive;
} sMPI;

struct ext_euc_res ext_euc(MPI a, MPI b) {
    // we have an issue - our bigint library supports
    // positive numbers only. This is fine in almost all
    // parts of the existing application, EXCEPT the extended
    // euclidean algorithm, where the calculated coefficents
    // and factors may go negative at any point during the
    // calculation. The returned bezout coefficients themselves
    // may also be negative.
    //
    // We can hack around our lack of negative number support.
    // However, there will be sign errors in the returned
    // coefficients. HOWEVER, if all you want to do with the
    // returned coefficients is put them through a modulus
    // operators (e.g. d = (bez coef x) mod lambda(n)), then
    // the sign is irrelevant.

    sMPI r = {NULL, true}, s = {NULL, true}, t = {NULL, true},
         old_r = {NULL, true}, old_s = {NULL, true}, old_t = {NULL, true},
         quotient = {NULL, true}, tmp1 = {NULL, true}, tmp2 = {NULL, true};

    old_r.val = bi_init_and_copy(a);
    r.val = bi_init_and_copy(b);
    old_s.val = bi_init_like(a);
    bi_set(old_s.val, 1);
    s.val = bi_init_like(a);
    old_t.val = bi_init_like(a);
    t.val = bi_init_like(a);
    bi_set(t.val, 1);

    while (!bi_eq_val(r.val, 0)) {
        quotient.val = bi_eucl_div(old_r.val, r.val);
        quotient.positive = old_r.positive ^ r.positive;

        // (old_r, r) := (r, old_r − quotient × r)
        tmp1.val = bi_init_and_copy(r.val);
        tmp1.positive = r.positive;

        tmp2.val = bi_mul(quotient.val, r.val);
        tmp2.positive = quotient.positive ^ r.positive;

        bi_free(r.val);

        r.val = bi_sub(old_r.val, tmp2.val);
        bi_free(old_r.val);
        old_r.val = bi_init_and_copy(tmp1.val);
        bi_free(tmp1.val);
        bi_free(tmp2.val);

        // (old_s, s) := (s, old_s − quotient × s)
        tmp1.val = bi_init_and_copy(s.val);
        tmp2.val = bi_mul(quotient.val, s.val);
        bi_free(s.val);
        s.val = bi_sub(old_s.val, tmp2.val);
        bi_free(old_s.val);
        old_s.val = bi_init_and_copy(tmp1.val);
        bi_free(tmp1.val);
        bi_free(tmp2.val);

        // (old_t, t) := (t, old_t − quotient × t)
        tmp1.val = bi_init_and_copy(t.val);
        tmp2.val = bi_mul(quotient.val, t.val);
        bi_free(t.val);
        t.val = bi_sub(old_t.val, tmp2.val);
        bi_free(old_t.val);
        old_t.val = bi_init_and_copy(tmp1.val);
        bi_free(tmp1.val);
        bi_free(tmp2.val);
        bi_free(quotient.val);
    }

    struct ext_euc_res res;
    res.bez_x = bi_init_and_copy(old_s.val);
    res.bez_y = bi_init_and_copy(old_t.val);
    res.gcd = bi_init_and_copy(old_r.val);

    bi_free(r.val);
    bi_free(s.val);
    bi_free(t.val);
    bi_free(old_r.val);
    bi_free(old_s.val);
    bi_free(old_t.val);
    return res;
}

struct lambda_n_d_res calc_lambda_n_d(MPI p, MPI q, MPI e) {
    struct lambda_n_d_res res;

    MPI p_cpy = bi_init_and_copy(p);
    bi_dec(p_cpy);
    MPI q_cpy = bi_init_and_copy(q);
    bi_dec(q_cpy);

    res.lambda_n = bi_lcm(p_cpy, q_cpy);

    struct ext_euc_res lcm_res = ext_euc(e, res.lambda_n);
    printf("calculating d, where lambda_n=\n");
    bi_printf(res.lambda_n);
    printf("\ne=\n");
    bi_printf(e);
    printf("\nlcm_res.bez_x=\n");
    bi_printf(lcm_res.bez_x);
    res.d = bi_mod(lcm_res.bez_x, res.lambda_n);

    bi_free(lcm_res.bez_x);
    bi_free(lcm_res.bez_y);
    bi_free(lcm_res.gcd);

    return res;
}

MPI gen_e(void) {
    MPI res = bi_init(1);
    bi_set(res, RSA_DEFAULT_E);
    return res;
}

void gen_pub_priv_keys(long seed, struct rsa_public_token *pub,
                       struct rsa_private_token *priv, rsa_mode_t mode) {

    struct rsa_state state;

    load_new_primes(&state, seed, mode);

    MPI n = bi_mul(state.p, state.q);
    MPI e = gen_e();

    struct lambda_n_d_res lambda_n_d = calc_lambda_n_d(state.p, state.q, e);

    pub->e = e;
    pub->n = n;

    priv->d = lambda_n_d.d;
    priv->n = n;

    bi_free(lambda_n_d.lambda_n);
}
