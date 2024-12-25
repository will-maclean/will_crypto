#ifndef __will_rng
#define __will_rng

#include "bigint.h"

struct will_rng_cfg{
	int words;
};

void init_will_rng(struct will_rng_cfg *cfg, unsigned int seed);
void will_rng_next(struct bigint **res);

#endif
