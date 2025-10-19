#include <rng/chacha.h>
#include <rng/rng.h>
#include <stdint.h>
#include <string.h>

struct will_rng_state {
    uint32_t prev[16];
};

static struct will_rng_state rng_state;

void will_rng_init(uint32_t seed) {
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

    MPI res = bi_init(words);

    if (words < 16) {
        uint32_t tmp[16] = {0};
        chacha_block(tmp, rng_state.prev);
        memcpy(rng_state.prev, tmp, 16 * sizeof(uint32_t));
        memcpy(res->data, tmp, words * sizeof(uint32_t));
        return res;
    }

    for (uint32_t i = 0; i < res->words; i += 16) {
        chacha_block(&(res->data[i]), rng_state.prev);
        memcpy(rng_state.prev, &(res->data[i]), 16 * sizeof(uint32_t));
    }

    if (words % 16 != 0) {
        uint32_t tmp[16] = {0};
        chacha_block(tmp, rng_state.prev);
        memcpy(rng_state.prev, tmp, 16 * sizeof(uint32_t));
        memcpy(&(res->data[words - (words % 16)]), tmp,
               (words % 16) * sizeof(uint32_t));
    }

    return res;
}
