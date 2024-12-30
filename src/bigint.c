#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	for(int i = 0; i < a->words - 1; i++)
		a->data[i] = 0u;
	
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
	if (!bi_ge(a, b)){
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
		/*
		printf("step: %d. a->data[i]=%u, a_copy->data[i]=%u, b->data[i]=%u, res->data[i]=%u\nres=",
				i, a->data[i], a_copy->data[i], b->data[i], (*res)->data[i]);
		bi_printf(*res);
		printf("\n");
		*/

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
			       	+ (*res)->data[i+j+1] + carry;
			// printf("i=%d, j=%d, prod=%lu\n", i, j, prod);
			(*res)->data[i+j+1] = (unsigned int)(prod & 0xFFFFFFFF);
			/*
			printf("res=");
			bi_printf(*res);
			printf("\n");
			*/
			carry = (unsigned int)(prod >> 32);
		}

		if(carry && i > 0){
			(*res)->data[i-1] += carry;
		}
	}
	
	unsigned int *tmp = malloc(a->words * sizeof(unsigned int));
	memcpy(tmp, &((*res)->data[a->words]), a->words * sizeof(unsigned int));
	free((*res)->data);
	(*res)->data = tmp;
	(*res)->words = a->words;
	
	return OKAY;
}
enum bi_op_result bi_mod(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	/* Using the multiple-precision division algorithm from 
	 * https://cacr.uwaterloo.ca/hac/about/chap14.pdf
	 */

	// First, find the used words of a and b
	// These are n and t, respectively.
	// For completeness, b is 2**32 (max unsigned int size)
	int n = a->words;
	int t = b->words;

	while(a->data[a->words - n] == 0u && n >0)
		n--;
	
	while(b->data[b->words - t] == 0u && t >0)
		t--;

	if(n == 0 || t == 0)
		return -EINVAL;

	// Initialise q and r
	struct bigint *q, r*;
	bi_init(&q, a->words);
	bi_init(&r, a->words);
	bi_set(q, 0u);

	struct bigint *x, *y;
	// We need a few constants throughout the computation
	struct bigint *b_pow_n_t, *tmp;
	

	/*	
	struct bigint *r, *tmp;

	bi_init_like(&r, a);
	bi_copy(a, r);

	while(bi_ge(r, b)){
		// r = r - d
		bi_sub(r, b, &tmp);
		bi_copy(tmp, r);
		bi_free(tmp);
	}

	(*res) = r;

	return OKAY;
	*/
}

enum bi_op_result bi_eucl_div(struct bigint *a, struct bigint *b,
		struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;
	
	struct bigint *r, *q, *tmp;

	bi_init_like(&r, a);
	bi_init_like(&q, a);

	bi_copy(a, r);
	bi_set(q, 0u);


	while(bi_ge(r, b)){
		
		// r = r - d
		bi_sub(r, b, &tmp);
		bi_copy(tmp, r);
		bi_free(tmp);

		// q = q + 1
		bi_inc(q);
	}

	(*res) = q;

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

bool bi_gt(struct bigint *a, struct bigint *b){
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
	return false;
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
void bi_inc(struct bigint *x)
{
	int i = x->words - 1;
	while(x->data[i] == 0xFFFFFFFF){
		x->data[i] = 0u;
		i--;
	}

	x->data[i]++;
}

void bi_dec(struct bigint *x)
{
	int i = x->words - 1;
	while(x->data[i] == 0u){
		x->data[i] = 0xFFFFFFFF;
		i--;
	}

	x->data[i]--;
}
enum bi_op_result bi_and(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init_like(res, a);

	for(int i = 0; i < a->words; i++){
		(*res)->data[i] = a->data[i] & b->data[i];
	}

	return OKAY;
}
enum bi_op_result bi_or(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init_like(res, a);

	for(int i = 0; i < a->words; i++){
		(*res)->data[i] = a->data[i] | b->data[i];
	}

	return OKAY;
}
enum bi_op_result bi_xor(struct bigint *a, struct bigint *b, struct bigint **res)
{
	if (!assert_same_shape(a, b))
		return -EBADSHAPE;

	bi_init_like(res, a);

	for(int i = 0; i < a->words; i++){
		(*res)->data[i] = a->data[i] ^ b->data[i];
	}

	return OKAY;
}
enum bi_op_result bi_not(struct bigint *a, struct bigint **res)
{
	bi_init_like(res, a);

	for(int i = 0; i < a->words; i++){
		(*res)->data[i] = ~a->data[i];
	}

	return OKAY;
}

enum bi_op_result bi_shift_left(struct bigint *a, struct bigint **res)
{
	printf("warning: bi_shift)right is not implemented\n");
	return OKAY;
}

enum bi_op_result bi_shift_right(struct bigint *a, struct bigint **res)
{
	printf("warning: bi_shift)right is not implemented\n");
	return OKAY;
}


// This is a very simple modular exponentiation algorithm. There are
// faster, more efficient algorithms that can be implemented later.
enum bi_op_result bi_mod_exp(struct bigint *x, struct bigint *exp, 
		struct bigint *mod, struct bigint **res)
{
	if(!assert_same_shape(x, exp))
		return -EBADSHAPE;

	if(!assert_same_shape(x, mod))
		return -EBADSHAPE;

	bi_init_like(res, x);

	if(bi_eq_val(mod, 1u)){
		bi_set(*res, 0u);
		return OKAY;	
	}
	
	bi_set(*res, 1u);
	
	// both the mod and the exp are bigints, which means
	// we have to use loop methods that support them
	
	struct bigint *loop_counter;
	bi_init_like(&loop_counter, x);
	bi_set(loop_counter, 0u);

	struct bigint *tmp;
	while(bi_lt(loop_counter, exp)){
		// res := (res * base) % mod

		bi_mul(*res, x, &tmp);
		bi_copy(tmp, *res);
		bi_free(tmp);
		
		bi_mod(*res, mod, &tmp);
		bi_copy(tmp, *res);
		bi_free(tmp);

		bi_inc(loop_counter);
	}

	return OKAY;	
}

bool bi_lt(struct bigint *a, struct bigint *b)
{
	if(a->words != b->words){
		// Honestly, not sure what to do here. I think for now I'll
		// just return false
		return false;
	}

	for(int i = 0; i < a->words; i++){
		if(a->data[i] > b->data[i]){
			return false;
		} else if (a->data[i] < b->data[i]){
			return true;
		}
	}

	// a and b are equal
	return false;
}

bool bi_le(struct bigint *a, struct bigint *b)
{
	if(a->words != b->words){
		// Honestly, not sure what to do here. I think for now I'll
		// just return false
		return false;
	}

	for(int i = 0; i < a->words; i++){
		if(a->data[i] > b->data[i]){
			return false;
		} else if (a->data[i] < b->data[i]){
			return true;
		}
	}

	// a and b are equal
	return true;
}

void bi_concat(struct bigint *a, struct bigint *b, struct bigint **res)
{
	(*res)->words = a->words + b->words;

	(*res)->data = malloc((*res)->words * sizeof(unsigned int));
	memcpy((*res)->data, a->data, a->words * sizeof(unsigned int));
	memcpy(&((*res)->data[a->words]), b->data, b->words * sizeof(unsigned int));
}

enum bi_op_result bi_init_and_copy(struct bigint *src, struct bigint **trgt)
{
	enum bi_op_result init_res = bi_init_like(trgt, src);

	if(init_res != OKAY)
		return init_res;
	bi_copy(src, *trgt);

	return OKAY;
}

bool bi_even(struct bigint *a)
{
	return !(a->data[a->words - 1] & 1u);
}
