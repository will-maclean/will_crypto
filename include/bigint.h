#ifndef __bigint
#define __bigint

#include <stdbool.h>

/*
 * struct bigint
 *
 * Stores some big numbers for us, and lets us do maths on them! We'll need this
 * for our crypto work, where we use e.g. 2048 bit tokens.
 *
 * Making the very brave decision that we're working in 64-bit environment. Not
 * sure if this will cause issues at any point, but I'm sure that's a problem
 * for future Will.
 */
struct bigint{
	int words;
	unsigned int *data;
};

/*
 * Assigns the memory for a bigint. This is the only function
 * that will raw assign memory for a bigint
 */
struct bigint *bi_init(int words);


/*
 * Initialises a bigint to have the same size/memory as
 * another bigint
 */
struct bigint *bi_init_like(struct bigint *like);

/*
 * Clears memory for a bigint. This is the only function that will clear mem
 * for a bigint.
 */
void bi_free(struct bigint *x);

/*
 * Copies the data from src into trgt. Assumes trgt's memory is already
 * initialised - if it isn't, use bi_init_and_copy instead
 */
void bi_copy(struct bigint *src, struct bigint *trgt);

/*
 * Initialises memory for trgt, and copes src into target
 */
struct bigint *bi_init_and_copy(struct bigint *src);

/*
 * Can be used in limited scenarios to set the bigint to a user-defined value
 * which can be stored in the int type for the sytem (again, assuming a 64-bit
 * arch.
 */
void bi_set(struct bigint *x, unsigned int val);

/*
 * All math function assume the result is NOT set
 */
struct bigint *bi_add(struct bigint *a, struct bigint *b);
struct bigint *bi_sub(struct bigint *a, struct bigint *b);
struct bigint *bi_mul(struct bigint *a, struct bigint *b);
void bi_inc(struct bigint *x);
void bi_dec(struct bigint *x);
struct bigint *bi_mod_exp(struct bigint *x, struct bigint *exp, 
		struct bigint *mod);
struct bigint *bi_powi(struct bigint *b, unsigned int p);

/*
 * remainder of a / b
 */
struct bigint *bi_mod(struct bigint *a, struct bigint *b);

/*
 * integer division result of a/b
 */
struct bigint *bi_eucl_div(struct bigint *a, struct bigint *b);

bool bi_eq(struct bigint *a, struct bigint* b);
bool bi_eq_val(struct bigint *a, unsigned int b);
bool bi_gt(struct bigint *a, struct bigint *b);
bool bi_ge(struct bigint *a, struct bigint *b);
bool bi_lt(struct bigint *a, struct bigint *b);
bool bi_le(struct bigint *a, struct bigint *b);
bool bi_even(struct bigint *a);

// Logical expressions
struct bigint *bi_and(struct bigint *a, struct bigint *b);
struct bigint *bi_or(struct bigint *a, struct bigint *b);
struct bigint *bi_xor(struct bigint *a, struct bigint *b);
struct bigint *bi_not(struct bigint *a);
struct bigint *bi_shift_left(struct bigint *a, unsigned int n);
struct bigint *bi_shift_right(struct bigint *a, unsigned int n);
void bi_printf(struct bigint *x);

// helper functions
struct bigint *bi_concat(struct bigint *a, struct bigint *b);
#endif
