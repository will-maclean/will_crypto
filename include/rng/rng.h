#ifndef __will_rng
#define __will_rng

#include <bigint/bigint.h>
#include <stdint.h>

void will_rng_init(uint32_t seed);
MPI will_rng_next(int words);
#endif
