#include "rsa.h"
#include "bigint.h"
#include "primality.h"
#include "rng.h"

void load_new_primes(struct rsa_state *new_state, uint32_t seed,
                     rsa_mode_t mode) {
    uint32_t prime_words;
    switch (mode) {
    case RSA_MODE_1024:
        prime_words = 16;
        break;
    }

    struct will_rng_cfg rng_cfg;
    rng_cfg.words = prime_words;

    init_will_rng(&rng_cfg, seed);

    new_state->p = gen_prime(prime_words);
    new_state->q = gen_prime(prime_words);
}

/*
 * Assumes bez_x and bez_y are NOT set
 */
struct lcm_ext_euc_res lcm_ext_euc(MPI a, MPI b) {
    MPI r, s, t, old_r, old_s, old_t, quotient, tmp1, tmp2;

    old_r = bi_init_and_copy(a);
    r = bi_init_and_copy(b);
    old_s = bi_init_like(a);
    bi_set(old_s, 1);
    s = bi_init_like(a);
    bi_set(s, 0);
    old_t = bi_init_like(a);
    bi_set(old_t, 0);
    t = bi_init_like(a);
    bi_set(t, 1);

    while (!bi_eq_val(r, 0)) {
        quotient = bi_eucl_div(old_r, r);

        // (old_r, r) := (r, old_r − quotient × r)
        tmp1 = bi_init_and_copy(r);
        tmp2 = bi_mul(quotient, r);
        bi_free(r);
        r = bi_sub(old_r, tmp2);
        bi_free(old_r);
        old_r = bi_init_and_copy(tmp1);
        bi_free(tmp1);
        bi_free(tmp2);

        // (old_s, s) := (s, old_s − quotient × s)
        tmp1 = bi_init_and_copy(s);
        tmp2 = bi_mul(quotient, s);
        bi_free(s);
        s = bi_sub(old_s, tmp2);
        bi_free(old_s);
        old_s = bi_init_and_copy(tmp1);
        bi_free(tmp1);
        bi_free(tmp2);

        // (old_t, t) := (t, old_t − quotient × t)
        tmp1 = bi_init_and_copy(t);
        tmp2 = bi_mul(quotient, t);
        bi_free(t);
        t = bi_sub(old_t, tmp2);
        bi_free(old_t);
        old_t = bi_init_and_copy(tmp1);
        bi_free(tmp1);
        bi_free(tmp2);
    }

    /* The Bézout coefficients are what we're here for. There's lots
     * of other information that can be extracted from old_r, t, and s,
     * but we don't care about it.
     */

    struct lcm_ext_euc_res res;
    res.bez_x = old_s;
    res.bez_y = old_t;
    res.lcm = old_r;
    return res;
}

struct lambda_n_d_res calc_lambda_n_d(MPI p, MPI q) {
    MPI p_cpy = bi_init_and_copy(p);
    bi_dec(p_cpy);
    MPI q_cpy = bi_init_and_copy(q);
    bi_dec(q_cpy);

    struct lcm_ext_euc_res lcm_res = lcm_ext_euc(p_cpy, q_cpy);
    struct lambda_n_d_res res;
    res.lambda_n = bi_init_and_copy(lcm_res.bez_x);
    res.d = bi_init_and_copy(lcm_res.bez_y);

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

    struct lambda_n_d_res lambda_n_d = calc_lambda_n_d(state.p, state.q);

    MPI e = gen_e();

    pub->e = e;
    pub->n = n;

    priv->d = lambda_n_d.d;
    priv->n = n;
}
