#ifndef __will_rng
#define __will_rng

#include "bigint.h"

struct will_rng_cfg{
	struct bigint *a;
	struct bigint *b;
	struct bigint *m;
	int words;
};

struct will_rng_state{
	struct will_rng_cfg *cfg;
	struct bigint *prev;
};

void init_will_rng(struct will_rng_cfg *cfg, unsigned int seed,
	      struct will_rng_state *new_state);
void will_rng_next(struct will_rng_state* state, struct bigint **res);

#endif
