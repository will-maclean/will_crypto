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

enum bi_op_result {
	OKAY = 0,
	EBADSHAPE = 1,
	ENOMEM = 2,
};

/*
 * Assigns the memory for a bigint. This is the only function
 * that will raw assign memory for a bigint
 */
enum bi_op_result bi_init(struct bigint **x, int words);


/*
 * Initialises a bigint to have the same size/memory as
 * another bigint
 */
enum bi_op_result bi_init_like(struct bigint **init, struct bigint *like);

/*
 * Clears memory for a bigint. This is the only function that will clear mem
 * for a bigint.
 */
void bi_free(struct bigint *x);

/*
 * Copies the data from src into trgt. Assumes trgt's memory is already
 * initialised - if it isn't, use bi_init_and_copy instead
 */
enum bi_op_result bi_copy(struct bigint *src, struct bigint *trgt);

/*
 * Initialises memory for trgt, and copes src into target
 */
enum bi_op_result bi_init_and_copy(struct bigint *src, struct bigint **trgt);

/*
 * Can be used in limited scenarios to set the bigint to a user-defined value
 * which can be stored in the int type for the sytem (again, assuming a 64-bit
 * arch.
 */
void bi_set(struct bigint *x, unsigned int val);

/*
 * All math function assume the result is NOT set
 */
enum bi_op_result bi_add(struct bigint *a, struct bigint *b, struct bigint **res);
enum bi_op_result bi_sub(struct bigint *a, struct bigint *b, struct bigint **res);
enum bi_op_result bi_mul(struct bigint *a, struct bigint *b, struct bigint **res);

/*
 * remainder of a / b
 */
enum bi_op_result bi_mod(struct bigint *a, struct bigint *b, struct bigint **res);

/*
 * integer division result of a/b
 */
enum bi_op_result bi_eucl_div(struct bigint *a, struct bigint *b, struct bigint **res);

bool bi_eq(struct bigint *a, struct bigint* b);
bool bi_eq_val(struct bigint *a, unsigned int b);
bool bi_ge(struct bigint *a, struct bigint *b);
void bi_printf(struct bigint *x);

#endif
