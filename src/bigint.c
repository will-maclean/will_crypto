#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"

static int assert_same_shape(struct bigint *a, struct bigint *b){
	return a->words == b->words;
}


enum bi_op_result bi_init(struct bigint **x, int words)
{
	*x = malloc(sizeof(struct bigint));

	if(!(*x))
		return -ENOMEM;

	(*x)->words = words;
	(*x)->data = malloc(words * sizeof(unsigned int));

	if(!(*x)->data)
		return -ENOMEM;

	return OKAY;
}

enum bi_op_result bi_init_like(struct bigint **init, struct bigint *like)
{
	return bi_init(init, like->words);
}

enum bi_op_result bi_copy(struct bigint *src, struct bigint *target){
	if (!assert_same_shape(src, target))
		return -EBADSHAPE;

	for(int i = 0; i < src->words; i++)
		target->data[i] = src->data[i];

	return OKAY;

}
void bi_free(struct bigint *x)
{
	free(x->data);
	free(x);
}

void bi_set(struct bigint *a, unsigned int val)
{
	a->data[a->words - 1] = val;
}

enum bi_op_result bi_add(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init(res, a->words);

	unsigned int carry = 0;
	unsigned long sum;
	for(int i = a->words - 1; i >= 0; i--){
		sum = (unsigned long)(a->data[i]) + (unsigned long)b->data[i]
			       	+ (*res)->data[i] + carry;
		(*res)->data[i] = (unsigned int)(sum & 0xFFFFFFFF);
		
		/*
		printf("step: %d. a->data[i]=%u, b->data[i]=%u, res->data[i]=%u, carry=%u. sum=%lu\nres=",
				i, a->data[i], b->data[i], (*res)->data[i], carry, sum);
		bi_printf(*res);
		printf("\n");
		*/

		carry = (unsigned int)(sum >> 32);

		if(carry && i > 0){
			(*res)->data[i - 1] += carry;
		}
	}

	return OKAY;
}

enum bi_op_result bi_sub(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	struct bigint *a_copy;
	bi_init_like(res, a);
	bi_init_like(&a_copy, a);
	bi_copy(a, a_copy);

	// we're working with unsigned ints, so if b is greater than a, we'll
	// set the result to 0 and return early
	if (bi_ge(a, b)){
		bi_set(*res, 0ul);
		return OKAY;
	}

	// TODO: future Will -> this can't be the best way to do this. Surely
	// we'll need to revisit this and make it smarter.

	unsigned long sub_from;
	for(int i = a->words - 1; i >= 0; i--){
		if(a_copy->data[i] >= b->data[i]){
			(*res)->data[i] = a_copy->data[i] - b->data[i];
		} else {
			// we need to go and get that bump
			int j = i - 1;

			// go to the left until we find something greater than
			// 0.
			while(j >= 0 && a_copy->data[j] == 0u)
				j--;

			// we found something greater then zero. We'll decrement
			// it and then carry the value back to the right again

			a_copy->data[j]--;

			// now move back to the right again, setting all the
			// zeros we find along the way to the max value (we know
			// they're zeros currently)
			j++;
			while(j!=i)
				a_copy->data[j] = 0xFFFFFFFF;

			// we can now compute the subtraction for the ith
			// place

			sub_from = (unsigned long) a_copy->data[i];
			sub_from += 1ul << 63;
			sub_from -= (unsigned long) b->data[i];
			sub_from &= 0xFFFFFFFF;
			(*res)->data[i] = (unsigned int) sub_from;
		}
	}

	/* underflow is UNDEFINED */

	return OKAY;
}
enum bi_op_result bi_mul(struct bigint *a, struct bigint *b, struct bigint **res)
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
			       	+ (*res)->data[2*a->words - i - j] + carry;
			(*res)->data[2*a->words - i -j] = (unsigned int)(prod & 0xFFFFFFFF);
			carry = (unsigned int)(prod >> 32);
		}

		if(carry){
			(*res)->data[2*a->words - i - b->words] += carry;
		}
	}

	unsigned int *old = (*res)->data;
	(*res)->data += a->words;
	free(old);

	return OKAY;
}
enum bi_op_result bi_mod(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init_like(res, a);

	unsigned long rem = 0;
	for(int i = 0; i < a->words; i++){
		// go from most sig to least sig
		rem = (unsigned long)(a) % (unsigned long)(b);
		(*res)->data[i] = rem;
		rem = rem >> 32;
	}

	return OKAY;
}

enum bi_op_result bi_eucl_div(struct bigint *a, struct bigint *b,
		struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init_like(res, a);

	unsigned long quot, rem = 0;
	for(int i = 0; i < a->words; i++){
		// go from most sig to least sig

		quot = ((unsigned long)(a) + rem) / (unsigned long)(b);
		rem = (unsigned long)(a) % (unsigned long)(b);
		rem = rem >> 32;

		(*res)->data[i] = quot;
	}

	return OKAY;

}

bool bi_eq(struct bigint *a, struct bigint *b)
{
	if(!assert_same_shape(a, b))
		return false;

	for(int i = 0; i < a->words; i++){
		if(a->data[i] != b->data[i])
			return false;
	}

	return true;
}

bool bi_eq_val(struct bigint *a, unsigned int b)
{
	for(int i = 0; i < a->words - 1; i++){
		if(a->data[i] != 0u)
			return false;
	}

	return a->data[a->words-1] == b;
}

bool bi_ge(struct bigint *a, struct bigint *b){
	if(a->words != b->words){
		// Honestly, not sure what to do here. I think for now I'll
		// just return false
		return false;
	}

	for(int i = 0; i < a->words; i++){
		if(a->data[i] > b->data[i]){
			return true;
		} else if (a->data[i] < b->data[i]){
			return false;
		}
	}

	// a and b are equal
	return true;
}

void bi_printf(struct bigint *x){
	printf("0x");
	for(int i = 0; i < x->words - 1; i++){
		printf("%08x_", x->data[i]);
	}
	printf("%08x", x->data[x->words-1]);

}
