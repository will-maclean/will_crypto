#ifndef __primality
#define __primality

#include <stdbool.h>
#include "bigint.h"

/*
 * Performs the Miller Rabin primality test on x. Completes k trials for primality
 */
bool miller_rabin(MPI x, int k);

#endif
