#ifndef __chacha
#define __chacha

#include <stdint.h>

void chacha_block(uint32_t out[16], uint32_t const in[16]);

#endif
