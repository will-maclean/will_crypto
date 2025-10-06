#ifndef __rsa
#define __rsa

#include "bigint.h"
#include <stdbool.h>

#define RSA_DEFAULT_E 65537

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

static void load_new_primes(struct rsa_state *new_state, int seed);

/*
 * Assumes bez_x and bez_y are NOT set
 */
int lcm_ext_euc(MPI a, MPI b, MPI bez_x, MPI bez_y);

static int calc_lambda_n_d(MPI p, MPI q, MPI lambda_n, struct bigint *d);

static int gen_e(MPI res);

void gen_pub_priv_keys(long seed, struct rsa_public_token *pub,
                       struct rsa_private_token *priv);

bool primality_test(MPI x);
#endif
