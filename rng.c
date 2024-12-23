#include <stdio.h>
#include <stdlib.h>
#include "rng.h"

void init_will_rng(struct will_rng_cfg *cfg, unsigned int seed,
	      struct will_rng_state *new_state)
{
	new_state->cfg = cfg;

	enum bi_op_result init_res = bi_init(&new_state->prev, cfg->words);

	if(init_res != OKAY){
		printf("Error code: %d", init_res);
		exit(1);
	}

	bi_set(new_state->prev, seed);
}

void will_rng_next(struct will_rng_state* state, struct bigint **res) {
	bi_mul(state->cfg->a, state->prev, res);
	bi_add(*res, state->cfg->b, res);
	bi_mod(*res, state->cfg->m, res);
	bi_copy(*res, state->prev);
}
