#ifndef __will_rng
#define __will_rng

#include "bigint.h"

struct will_rng_cfg{
	int words;
};

void init_will_rng(struct will_rng_cfg *cfg, unsigned int seed);
struct bigint *will_rng_next();
int get_rng_words();
#endif
