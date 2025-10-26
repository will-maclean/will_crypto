#ifndef __rsa
#define __rsa

#include <bigint/bigint.h>
#include <stdbool.h>

#define RSA_DEFAULT_E 65537

typedef enum {
    RSA_MODE_512,
    RSA_MODE_1024,
    RSA_MODE_2048,
    RSA_MODE_4096,
} rsa_mode_t;

void rsa_mode_to_str(rsa_mode_t mode, char *res);

// returns 0 if parse success, 1 otherwise
int rsa_mode_from_str(char *str, rsa_mode_t *res);

typedef struct {
    MPI e;
    MPI n;
} rsa_public_token_t;

typedef struct {
    MPI d;
    MPI n;
} rsa_private_token_t;

typedef struct {
    MPI p;
    MPI q;
} rsa_state_t;

void load_new_primes(rsa_state_t *new_state, uint32_t seed, rsa_mode_t mode,
                     MPI e);

typedef struct {
    MPI lambda_n;
    MPI d;
} lambda_n_d_res_t;
lambda_n_d_res_t calc_lambda_n_d(MPI p, MPI q, MPI e);

MPI gen_e(void);

void gen_pub_priv_keys(long seed, rsa_public_token_t *pub,
                       rsa_private_token_t *priv, rsa_mode_t mode);

void pub_key_to_file(rsa_public_token_t *pub, char *path, rsa_mode_t mode,
                     bool overwrite_existing);
void priv_key_to_file(rsa_private_token_t *pub, char *path, rsa_mode_t mode,
                      bool overwrite_existing);
void pub_key_from_file(rsa_public_token_t *pub, char *path);
void priv_key_from_file(rsa_private_token_t *pub, char *path);

MPI will_rsa_encrypt_num(MPI input, rsa_public_token_t *key);
MPI will_rsa_decrypt_num(MPI input, rsa_private_token_t *key);
#endif
