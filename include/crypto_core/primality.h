#ifndef __primality
#define __primality

#include <bigint/bigint.h>
#include <stdbool.h>

struct mr_sd {
    MPI s;
    MPI d;
};

struct mr_sd miller_rabin_sd(MPI n);

bool __miller_rabin_inner_check(MPI n, MPI a, struct mr_sd sd);

/*
 * Performs the Miller Rabin primality test on x. Completes k trials for
 * primality
 */
bool miller_rabin(MPI x, int k);

MPI gen_prime(int words);
#endif
