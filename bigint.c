#include "bigint.h"
#include <stdlib.h>

static int assert_same_shape(struct bigint *a, struct bigint *b){
	return a->words == b->words;
}


enum bi_op_result bi_init(struct bigint *x, int words)
{
	x->words = words;
	x->data = malloc(words * sizeof(unsigned int));

	if(!x->data)
		return -ENOMEM;

	return OKAY;
}

void bi_free(struct bigint *x)
{
	free(x->data);
}

void bi_set(struct bigint *a, unsigned int val)
{
	a->data[a->words] = val;
}

enum bi_op_result bi_add(struct bigint *a, struct bigint *b, struct bigint *res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init(res, 2*a->words);

	unsigned int carry;
	unsigned long sum;
	for(int i = a->words - 1; i >= 0; i--){
		sum = (unsigned long)(a->data[i]) + (unsigned long)b->data[i]
			       	+ res->data[i] + carry;
		res->data[i] = (unsigned int)(sum & 0xFFFFFFFF);
		carry = (unsigned int)(sum >> 32);

		if(carry){
			res->data[i - 1] += carry;
		}
	}

	unsigned int *old = res->data;
	res->data += a->words;
	free(old);

	return OKAY;
}

enum bi_op_result bi_sub(struct bigint *a, struct bigint *b, struct bigint *res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init_like(res, a);

	int carry_in = 0u, carry_out;
	for(int i = 0; i < a->words; i--){
		res->data[i] = __builtin_subc(a->data[i], b->data[i],
				carry_in, &carry_out);

		carry_in = carry_out;
	}

	/* underflow is UNDEFINED */

	return OKAY;
}
enum bi_op_result bi_mul(struct bigint *a, struct bigint *b, struct bigint *res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init(res, 2*a->words);

	unsigned int carry;
	unsigned long prod;
	for(int i = a->words - 1; i >= 0; i--){
		carry = 0;
		for(int j = b->words - 1; j >= 0; j--){
			prod = (unsigned long)(a->data[i]) * (unsigned long)b->data[j]
			       	+ res->data[2*a->words - i - j] + carry;
			res->data[2*a->words - i -j] = (unsigned int)(prod & 0xFFFFFFFF);
			carry = (unsigned int)(prod >> 32);
		}

		if(carry){
			res->data[2*a->words - i - b->words] += carry;
		}
	}

	unsigned int *old = res->data;
	res->data += a->words;
	free(old);

	return OKAY;
}
enum bi_op_result bi_mod(struct bigint *a, struct bigint *b, struct bigint *res)
{
	return OKAY;
}

