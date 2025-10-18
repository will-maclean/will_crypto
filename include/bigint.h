#ifndef __bigint
#define __bigint

#include <stdbool.h>
#include <stdint.h>

struct bigint {
    uint32_t words;
    uint32_t *data;
};

typedef struct bigint *MPI;

/*
 * Assigns the memory for a bigint. This is the only function
 * that will raw assign memory for a bigint
 */
MPI bi_init(uint32_t words);

/*
 * Initialises a bigint to have the same size/memory as
 * another bigint
 */
MPI bi_init_like(MPI like);

/*
 * Clears memory for a bigint. This is the only function that will clear mem
 * for a bigint.
 */
void bi_free(MPI x);

/*
 * Copies the data from src into trgt. Assumes trgt's memory is already
 * initialised - if it isn't, use bi_init_and_copy instead
 */
void bi_copy(MPI src, MPI trgt);

/*
 * Initialises memory for trgt, and copes src into target
 */
MPI bi_init_and_copy(MPI src);

/*
 * Can be used in limited scenarios to set the bigint to a user-defined value
 * which can be stored in the int type for the sytem (again, assuming a 64-bit
 * arch.
 */
void bi_set(MPI x, uint32_t val);

/*
 * All math function assume the result is NOT set
 */

MPI bi_add(MPI a, MPI b);

// a += b
void add_in_place(MPI a, MPI b);

MPI bi_sub(MPI a, MPI b);

// a -= b
void bi_sub_in_place(MPI a, MPI b);

MPI bi_mul(MPI a, MPI b);
MPI bi_mul_imm(MPI a, uint32_t x);
void bi_inc(MPI x);
void bi_dec(MPI x);
MPI bi_mod_exp(MPI x, MPI exp, MPI mod);
MPI bi_pow_imm(MPI b, uint32_t p);

MPI knuth_d(MPI u, MPI v, bool return_quotient);
/*
 * remainder of a / b
 */
MPI bi_mod(MPI a, MPI b);
MPI bi_mod_imm(MPI a, uint32_t b);

/*
 * integer division result of a/b
 */
MPI bi_eucl_div(MPI a, MPI b);
MPI bi_eucl_div_imm(MPI a, uint32_t b);

bool bi_eq(MPI a, MPI b);
bool bi_eq_val(MPI a, uint32_t b);
bool bi_gt(MPI a, MPI b);
bool bi_ge(MPI a, MPI b);
bool bi_lt(MPI a, MPI b);
bool bi_le(MPI a, MPI b);
bool bi_even(MPI a);

// Logical expressions
MPI bi_and(MPI a, MPI b);
MPI bi_or(MPI a, MPI b);
MPI bi_xor(MPI a, MPI b);
MPI bi_not(MPI a);
MPI bi_shift_left(MPI a, uint32_t n);
MPI bi_shift_right(MPI a, uint32_t n);
void bi_printf(MPI x);

// helper functions
MPI bi_concat(MPI a, MPI b);
MPI bi_slice(MPI a, uint32_t start, uint32_t end);
void bi_copy_word_range(MPI src, MPI target, uint32_t src_start_idx,
                        uint32_t target_start_idx, uint32_t copy_words);
void bi_add_to_range(MPI src, MPI target, uint32_t src_start_idx,
                     uint32_t target_start_idx, uint32_t range_words);

/*
 * Pads x with 0's
 */
MPI bi_pad_words(MPI x, uint32_t n);
MPI bi_pad_words_from_bottom(MPI x, uint32_t n);
void bi_squeeze(MPI x);

#endif
