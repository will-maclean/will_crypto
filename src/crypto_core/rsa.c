#include <bigint/bigint.h>
#include <crypto_core/primality.h>
#include <crypto_core/rsa.h>
#include <rng/rng.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rsa_mode_to_str(rsa_mode_t mode, char *res) {
    switch (mode) {
    case RSA_MODE_512:
        strcpy(res, "will_rsa_512");
        break;
    case RSA_MODE_1024:
        strcpy(res, "will_rsa_1024");
        break;
    case RSA_MODE_2048:
        strcpy(res, "will_rsa_2048");
        break;
    case RSA_MODE_4096:
        strcpy(res, "will_rsa_4096");
        break;
    }
}

void load_new_primes(rsa_state_t *new_state, uint32_t seed, rsa_mode_t mode,
                     MPI e) {
    uint32_t prime_words;
    switch (mode) {
    case RSA_MODE_1024:
        prime_words = 16;
        break;
    case RSA_MODE_512:
        prime_words = 8;
        break;
    case RSA_MODE_2048:
        prime_words = 32;
        break;
    case RSA_MODE_4096:
        prime_words = 64;
        break;
    }

    will_rng_init(seed);

    MPI p, q;
    uint32_t max_trials = 10000;

    for (uint32_t i = 0; i < max_trials; i++) {
        p = gen_prime(prime_words);
        q = gen_prime(prime_words);

        // ensure gcd(p-1, e) = 1 and gcd(q-1, e) = 1
        bi_dec(p);
        bi_dec(q);

         ext_euc_res_t p_gcd = ext_euc(p, e);

        if (!signed_eq_val(p_gcd.gcd, 1, true)) {
            // p-1 is not coprime with e -> try again
            bi_free(p);
            bi_free(q);
            signed_free(p_gcd.bez_x);
            signed_free(p_gcd.bez_y);
            signed_free(p_gcd.gcd);
            continue;
        }

        ext_euc_res_t q_gcd = ext_euc(q, e);

        if (!signed_eq_val(q_gcd.gcd, 1, true )) {
            // q-1 is not coprime with e -> try again
            bi_free(p);
            bi_free(q);
            signed_free(p_gcd.bez_x);
            signed_free(p_gcd.bez_y);
            signed_free(p_gcd.gcd);
            continue;
        } else {
            // both p-1 and q-1 are coprime with e -> acceptable values
            signed_free(p_gcd.bez_x);
            signed_free(p_gcd.bez_y);
            signed_free(p_gcd.gcd);

            bi_inc(p);
            bi_inc(q);

            new_state->p = p;
            new_state->q = q;

            return;
        }
    }

    new_state->p = gen_prime(prime_words);
    new_state->q = gen_prime(prime_words);
}



lambda_n_d_res_t calc_lambda_n_d(MPI p, MPI q, MPI e) {
    lambda_n_d_res_t res;

    MPI p_cpy = bi_init_and_copy(p);
    bi_dec(p_cpy);
    MPI q_cpy = bi_init_and_copy(q);
    bi_dec(q_cpy);

    res.lambda_n = bi_lcm(p_cpy, q_cpy);
    res.d = bi_mod_mult_inv(e, res.lambda_n);
    
    bi_free(p_cpy);
    bi_free(q_cpy);

    return res;
}

MPI gen_e(void) {
    MPI res = bi_init(1);
    bi_set(res, RSA_DEFAULT_E);
    return res;
}

void gen_pub_priv_keys(long seed, rsa_public_token_t *pub,
                       rsa_private_token_t *priv, rsa_mode_t mode) {

    rsa_state_t state;

    MPI e = gen_e();
    load_new_primes(&state, seed, mode, e);

    MPI n = bi_mul(state.p, state.q);

    lambda_n_d_res_t lambda_n_d = calc_lambda_n_d(state.p, state.q, e);

    // assertion for safety
    // gcd(lambda(n), e) = 1
    ext_euc_res_t check = ext_euc(lambda_n_d.lambda_n, e);
    if(!signed_eq_val(check.gcd, 1, true)){
        printf("ERROR in RSA keygen: gcd(lambda(n), e) != 1. n=");
        bi_print(n);
        printf("\ne=");
        bi_print(e);
        printf("\nlambda(n)=");
        bi_print(lambda_n_d.lambda_n);
    }

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
void pub_key_to_file(rsa_public_token_t *pub, char *path, rsa_mode_t mode,
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
        char name[256];
        rsa_mode_to_str(mode, name);

        bi_printf(pub->n, fp);
        fprintf(fp, "\n");
        bi_printf(pub->e, fp);
        fprintf(fp, "\n%s(public)\n", name);
        fclose(fp);

    } else {
        printf("Unable to open public key file %s. Private.", path);
        exit(1);
    }
}

void pub_key_from_file(rsa_public_token_t *pub, char *path) {
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
void priv_key_to_file(rsa_private_token_t *priv, char *path, rsa_mode_t mode,
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
        char name[256];
        rsa_mode_to_str(mode, name);

        bi_printf(priv->n, fp);
        fprintf(fp, "\n");
        bi_printf(priv->d, fp);
        fprintf(fp, "\n%s(private)\n", name);

        fclose(fp);

    } else {
        printf("Unable to open private key file %s. Exiting.", path);
        exit(1);
    }
}

void priv_key_from_file(rsa_private_token_t *priv, char *path) {
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

MPI will_rsa_encrypt_num(MPI input, rsa_public_token_t *key) {
    return bi_mod_exp(input, key->e, key->n);
}

MPI will_rsa_decrypt_num(MPI input, rsa_private_token_t *key) {
    return bi_mod_exp(input, key->d, key->n);
}
