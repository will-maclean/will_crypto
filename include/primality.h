#ifndef __primality
#define __primality

#include "bigint.h"
#include <stdbool.h>

/*
 * Performs the Miller Rabin primality test on x. Completes k trials for
 * primality
 */
bool miller_rabin(MPI x, int k);

MPI gen_prime(int words);
#endif
