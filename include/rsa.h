#ifndef __rsa
#define __rsa

#include "bigint.h"
#include <stdbool.h>

#define RSA_DEFAULT_E 65537

typedef enum {
    RSA_MODE_1024,
} rsa_mode_t;

struct rsa_public_token {
    MPI e;
    MPI n;
};

struct rsa_private_token {
    MPI d;
    MPI n;
};

struct rsa_state {
    MPI p;
    MPI q;
};

void load_new_primes(struct rsa_state *new_state, uint32_t seed,
                     rsa_mode_t mode);

struct lcm_ext_euc_res {
    MPI bez_x;
    MPI bez_y;
    MPI lcm;
};

struct lcm_ext_euc_res lcm_ext_euc(MPI a, MPI b);

struct lambda_n_d_res {
    MPI lambda_n;
    MPI d;
};
struct lambda_n_d_res calc_lambda_n_d(MPI p, MPI q);

MPI gen_e(void);

void gen_pub_priv_keys(long seed, struct rsa_public_token *pub,
                       struct rsa_private_token *priv, rsa_mode_t mode);

#endif
