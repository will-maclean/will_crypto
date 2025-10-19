#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <crypto_core/rsa.h>
#include <rng/rng.h>
#include <stdio.h>
#include <stdlib.h>

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

    will_rng_init(seed);

    new_state->p = gen_prime(prime_words);
    new_state->q = gen_prime(prime_words);
}

typedef struct {
    MPI val;
    bool positive;
} sMPI;

void signed_div(sMPI a, sMPI b, sMPI *res) {
    res->val = bi_eucl_div(a.val, b.val);
    res->positive = a.positive ^ b.positive;
}

void signed_sub(sMPI a, sMPI b, sMPI *res) {
    if (a.positive == b.positive) {
        if (bi_gt(a.val, b.val)) {
            res->val = bi_sub(a.val, b.val);
            res->positive = a.positive;
        } else {
            res->val = bi_sub(b.val, a.val);
            res->positive = !a.positive;
        }
    } else {
        res->val = bi_add(a.val, b.val);
        res->positive = a.positive;
    }
}

void signed_mul(sMPI a, sMPI b, sMPI *res) {
    res->val = bi_mul(a.val, b.val);
    res->positive = a.positive ^ b.positive;
}

void signed_init_copy(sMPI src, sMPI *target) {
    target->val = bi_init_and_copy(src.val);
    target->positive = src.positive;
}

void signed_free_init_copy(sMPI src, sMPI *target) {
    bi_free(target->val);
    signed_init_copy(src, target);
}

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
        signed_div(old_r, r, &quotient);

        // (old_r, r) := (r, old_r − quotient × r)
        signed_init_copy(r, &tmp1);
        signed_mul(quotient, r, &tmp2);
        bi_free(r.val);
        signed_sub(old_r, tmp2, &r);
        signed_free_init_copy(tmp1, &old_r);
        bi_free(tmp1.val);
        bi_free(tmp2.val);

        // (old_s, s) := (s, old_s − quotient × s)
        signed_init_copy(s, &tmp1);
        signed_mul(quotient, s, &tmp2);
        bi_free(s.val);
        signed_sub(old_s, tmp2, &s);
        signed_free_init_copy(tmp1, &old_s);
        bi_free(tmp1.val);
        bi_free(tmp2.val);

        // (old_t, t) := (t, old_t − quotient × t)
        signed_init_copy(t, &tmp1);
        signed_mul(quotient, t, &tmp2);
        bi_free(t.val);
        signed_sub(old_t, tmp2, &t);
        signed_free_init_copy(tmp1, &old_t);
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

    struct ext_euc_res euc_res = ext_euc(p_cpy, q_cpy);
    MPI lcm_tmp1 = bi_eucl_div(p_cpy, euc_res.gcd);
    res.lambda_n = bi_mul(lcm_tmp1, q_cpy);

    bi_free(euc_res.bez_x);
    bi_free(euc_res.bez_y);
    bi_free(euc_res.gcd);

    euc_res = ext_euc(e, res.lambda_n);
    res.d = bi_mod(euc_res.bez_x, res.lambda_n);
    bi_squeeze(res.d);

    bi_free(euc_res.bez_x);
    bi_free(euc_res.bez_y);
    bi_free(euc_res.gcd);

    bi_free(p_cpy);
    bi_free(q_cpy);
    bi_free(lcm_tmp1);

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
    priv->n = bi_init_and_copy(n);

    bi_free(lambda_n_d.lambda_n);
    bi_free(state.p);
    bi_free(state.q);
}

bool file_exists(char *path) {
    FILE *fp;
    fp = fopen(path, "r");

    if (fp) {
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

// public key file standard will be simple: numbers stored in hex format, n on
// first line, e on second line
void pub_key_to_file(struct rsa_public_token *pub, char *path,
                     bool overwrite_existing) {
    if (file_exists(path) && !overwrite_existing) {
        printf(
            "Public key file %s already exists! Exiting without overwriting\n",
            path);
        exit(1);
    }

    FILE *fp;
    fp = fopen(path, "w");

    if (fp) {

        bi_printf(pub->n, fp);
        fprintf(fp, "\n");
        bi_printf(pub->e, fp);
        fprintf(fp, "\nwill_rsa(public)\n");
        fclose(fp);

    } else {
        printf("Unable to open public key file %s. Private.", path);
        exit(1);
    }
}

void pub_key_from_file(struct rsa_public_token *pub, char *path) {
    if (!file_exists(path)) {
        printf("Public key file %s does not exist! Exiting.\n", path);
        exit(1);
    }

    FILE *fp;
    fp = fopen(path, "r");

    if (fp) {
        char n_str[4096];
        char e_str[4096];
        fscanf(fp, "%s\n%s\n", n_str, e_str);

        pub->n = from_hex_str(n_str);
        pub->e = from_hex_str(e_str);
        fclose(fp);
    } else {
        printf("Unable to open public key file %s", path);
        exit(1);
    }
}

// private key file standard will be simple: numbers stored in hex format, n on
// first line, d on second line
void priv_key_to_file(struct rsa_private_token *priv, char *path,
                      bool overwrite_existing) {
    if (file_exists(path) && !overwrite_existing) {
        printf(
            "Private key file %s already exists! Exiting without overwriting\n",
            path);
        exit(1);
    }

    FILE *fp;
    fp = fopen(path, "w");

    if (fp) {

        bi_printf(priv->n, fp);
        fprintf(fp, "\n");
        bi_printf(priv->d, fp);
        fprintf(fp, "\nwill_rsa(private)\n");

        fclose(fp);

    } else {
        printf("Unable to open private key file %s. Exiting.", path);
        exit(1);
    }
}

void priv_key_from_file(struct rsa_private_token *priv, char *path) {
    if (!file_exists(path)) {
        printf("Private key file %s does not exist! Exiting.\n", path);
        exit(1);
    }

    FILE *fp;
    fp = fopen(path, "r");

    if (fp) {
        char n_str[4096];
        char d_str[4096];
        fgets(n_str, sizeof(n_str), fp);
        fgets(d_str, sizeof(d_str), fp);

        priv->n = from_hex_str(n_str);
        priv->d = from_hex_str(d_str);

        fclose(fp);
    } else {
        printf("Unable to open public key file %s", path);
        exit(1);
    }
}
