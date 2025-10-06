#ifndef __will_rng
#define __will_rng

#include "bigint.h"
#include <stdint.h>

struct will_rng_cfg {
    int words;
};

void init_will_rng(struct will_rng_cfg *cfg, uint32_t seed);
MPI will_rng_next(int words);
#endif
