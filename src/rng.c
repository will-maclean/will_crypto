#include <stdio.h>
#include <stdlib.h>
#include "rng.h"
#include "chacha.h"

struct will_rng_state{
	struct will_rng_cfg *cfg;
	struct bigint *prev;
};

static struct will_rng_state rng_state;

void init_will_rng(struct will_rng_cfg *cfg, unsigned int seed)
{
	// We use chacha for rng. chacha generates 512bit numbers.
	// We can therefore generate numbers which are multiples of
	// 512 bits - or 16 words.
	if (cfg->words % 16 != 0){
		printf("Can only generate numbers that are multiples of 512 bits (or 16 words) long\n");
		exit(1);
	}

	rng_state.cfg = cfg;
	rng_state.prev = bi_init(cfg->words);

	// Probably not the most secure seeding method...
	for(int i = 0; i < rng_state.prev->words; i++)
		rng_state.prev->data[i] = i * seed;

}

struct bigint *will_rng_next() 
{
	/*
	 * Idea is as follows:
	 *
	 * 1. For each 512 bit in the output block:
	 * 	1. Get the corresponding 512 bit block from rng_state->prev
	 * 	2. iterate chacha over it
	 * 	3. Set the chacha output to that same 512 bit block in rng_state->prev, as
	 * 	well as the matching block in *res
	 */
	struct bigint *res = bi_init_like(rng_state.prev);

	for(int i = 0; i < res->words; i += 16){
		chacha_block(&(res->data[i]), &(rng_state.prev->data[i]));
	}

	bi_copy(res, rng_state.prev);

	return res;
}

int get_rng_words(){
	return rng_state.cfg->words;
}
