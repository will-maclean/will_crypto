#include "rng.h"
#include "chacha.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct will_rng_state {
    struct will_rng_cfg *cfg;
    uint32_t prev[16];
};

static struct will_rng_state rng_state;

void init_will_rng(struct will_rng_cfg *cfg, uint32_t seed) {
    // We use chacha for rng. chacha generates 512bit numbers.
    // We can therefore generate numbers which are multiples of
    // 512 bits - or 16 words.
    if (cfg->words % 16 != 0) {
        printf("Can only generate numbers that are multiples of 512 bits (or "
               "16 words) long\n");
        exit(1);
    }

    rng_state.cfg = cfg;

    // Probably not the most secure seeding method...
    for (int i = 0; i < 16; i++)
        rng_state.prev[i] = i * seed;
}

MPI will_rng_next(int words) {
    /*
     * Idea is as follows:
     *
     * 1. For each 512 bit in the output block:
     * 	1. Get the corresponding 512 bit block from rng_state->prev
     * 	2. iterate chacha over it
     * 	3. Set the chacha output to that same 512 bit block in rng_state->prev,
     * as well as the matching block in *res
     */
    if (words % 16 != 0) {
        printf("Can only generate numbers that are multiples of 512 bits (or "
               "16 words) long\n");
        exit(1);
    }

    MPI res = bi_init(words);

    for (int i = 0; i < res->words; i += 16) {
        chacha_block(&(res->data[i]), rng_state.prev);
        memcpy(rng_state.prev, &(res->data[i]), 16 * sizeof(uint32_t));
    }

    return res;
}
