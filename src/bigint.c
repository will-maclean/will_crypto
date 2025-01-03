#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bigint.h"

int assert_same_shape(MPI a, MPI b){
	return a->words == b->words;
}

int min(int a, int b){
	if(a < b) {
		return a;
	} else {
		return b;
	}
}

int max(int a, int b){
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

MPI bi_init(int words)
{
	MPI x = malloc(sizeof(struct bigint));

	x->words = words;
	x->data = malloc(words * sizeof(unsigned int));

	return x;
}

MPI bi_init_like(MPI like)
{
	return bi_init(like->words);
}

void bi_copy(MPI src, MPI target){
	free(target->data);
	target->data = malloc(src->words * sizeof(unsigned int));
	memcpy(target->data, src->data, src->words * sizeof(unsigned int));
}

void bi_free(MPI x)
{
	free(x->data);
	free(x);
}

void bi_set(MPI a, unsigned int val)
{
	free(a->data);
	a->data = malloc(sizeof(unsigned int));
	a->data[0] = val;
	a->words = 1;
}

MPI bi_add(MPI a, MPI b)
{
	MPI res = bi_init(max(a->words, b->words) + 1);

	unsigned int carry = 0;
	unsigned long sum;
	for(int i = 0; i < min(a->words, b->words); i++){
		sum = (unsigned long)(a->data[i]) + (unsigned long)b->data[i]
			       	+ res->data[i] + carry;
		res->data[i] = (unsigned int)(sum & 0xFFFFFFFF);

		/*
		printf("step: %d. a->data[i]=%u, b->data[i]=%u, res->data[i]=%u, carry=%u. sum=%lu\nres=",
				i, a->data[i], b->data[i], (*res)->data[i], carry, sum);
		bi_printf(*res);
		printf("\n");
		*/

		carry = (unsigned int)(sum >> 32);

		if(carry && i > 0){
			res->data[i + 1] += carry;
		}
	}

	if(a->words > b->words){
		res->data[b->words] = carry + a->data[b->words];
		for(int i = b->words + 1; i < a->words; i++){
			res->data[i] = a->data[i];
		}
	} else if(b->words > a->words) {
		res->data[a->words] = carry + b->data[a->words];
		for(int i = a->words + 1; i < b->words; i++){
			res->data[i] = b->data[i];
		}
	} else {
		res->data[res->words - 1] = carry;
	}

	bi_squeeze(res);
	return res;
}

MPI bi_sub(MPI a, MPI b)
{
	MPI res = bi_init_like(a);
	MPI a_copy = bi_init_like(a);
	bi_copy(a, a_copy);

	// we're working with unsigned ints, so if b is greater than a, we'll
	// set the result to 0 and return early
	if (!bi_ge(a, b)){
		bi_set(res, 0u);
		return res;
	}

	// TODO: future Will -> this can't be the best way to do this. Surely
	// we'll need to revisit this and make it smarter.

	unsigned long sub_from;
	for(int i = 0; i < b->words; i++){
		if(a_copy->data[i] >= b->data[i]){
			res->data[i] = a_copy->data[i] - b->data[i];
		} else {
			// we need to go and get that bump
			int j = i + 1;

			// go to the left until we find something greater than
			// 0.
			while(j >= 0 && a_copy->data[j] == 0u)
				j++;

			// we found something greater then zero. We'll decrement
			// it and then carry the value back to the right again

			a_copy->data[j]--;

			// now move back to the right again, setting all the
			// zeros we find along the way to the max value (we know
			// they're zeros currently)
			j--;
			while(j!=i){
				a_copy->data[j] = 0xFFFFFFFF;
				j--;
			}

			// we can now compute the subtraction for the ith
			// place

			sub_from = (unsigned long) a_copy->data[i];
			sub_from += 1ul << 63;
			sub_from -= (unsigned long) b->data[i];
			sub_from &= 0xFFFFFFFF;
			res->data[i] = (unsigned int) sub_from;
		}
		/*
		printf("step: %d. a->data[i]=%u, a_copy->data[i]=%u, b->data[i]=%u, res->data[i]=%u\nres=",
				i, a->data[i], a_copy->data[i], b->data[i], (*res)->data[i]);
		bi_printf(*res);
		printf("\n");
		*/

	}

	for(int i = b->words; i < res->words; i++){
		res->data[i] = a->data[i];
	}

	bi_squeeze(res);
	return res;
}
MPI bi_mul(MPI a, MPI b)
{
	MPI res = bi_init(2*max(a->words, b->words));

	unsigned int carry;
	unsigned long prod;
	for(int i = 0; i < a->words; i++){
		carry = 0u;
		for(int j = 0; j < b->words; j++){
			prod = (unsigned long)(a->data[i]) * (unsigned long)b->data[j]
			       	+ res->data[i+j+1] + carry;
			// printf("i=%d, j=%d, prod=%lu\n", i, j, prod);
			res->data[i+j] = (unsigned int)(prod & 0xFFFFFFFF);
			/*
			printf("res=");
			bi_printf(*res);
			printf("\n");
			*/
			carry = (unsigned int)(prod >> 32);
		}

		if(carry){
			res->data[i+1] += carry;
		}
	}

	bi_squeeze(res);

	return res;
}

MPI bi_powi(MPI b, unsigned int p)
{
	// Basic exponentiation algorithm, plenty of faster ones 
	// out there if required. I don't think this can take
	// more then log2(p) iterations, which is pretty good,
	// so we should be safe for a while.

	MPI a = bi_init(2 * b->words);
	bi_set(a, 1u);
	MPI s = bi_init(2 * b->words);

	for(int i = 0; i < b->words; i++){
		s->data[i] = b->data[i];
	}

	while(p != 0u){
		if(p % 2 == 1){
			MPI tmp = bi_mul(a, s);
			bi_copy(tmp, a);
			bi_free(tmp);
		}

		p /= 2;

		if(p != 0){
			MPI tmp = bi_mul(s, s);
			bi_copy(tmp, s);
			bi_free(tmp);
		}
	}

	bi_free(s);

	return a;
}

// x % y
MPI bi_mod(MPI x, MPI y)
{
	//TODO: handle div by zero

	MPI r = bi_init_like(x);
	bi_set(r, 0u);

	int div_bits = 32u * x->words;
	for(int i = div_bits - 1; i >= 0; i--){
		// Left-shift R by 1 bit
		MPI tmp = bi_shift_left(r, 1u);
		bi_copy(tmp, r);
		bi_free(tmp);

		// Set the least-significant bit of R equal to bit i of the numerator
		unsigned int curr_word = i / 32u;
		unsigned int curr_word_pos = i % 32u;
		unsigned int res = x->data[curr_word] & (1u << curr_word_pos);
		r->data[0] |= res >> curr_word_pos;

		if(bi_ge(r, y)){
			// r := r - y
			tmp = bi_sub(r, y);
			bi_copy(tmp, r);
			bi_free(tmp);
		}
	}
	
	bi_squeeze(r);
	return r;

	/*
	// Implementing the multiple-precision division algorithm from
	// https://cacr.uwaterloo.ca/hac/about/chap14.pdf

	int n = x->words - 1;
	int t = y->words - 1;

	MPI q = bi_init_like(x);
	bi_set(q, 0u);
	MPI r = bi_init_and_copy(x);

	// b = base = 2**32
	MPI b = bi_init_like(x);
	b->data[2] = 1u;

	MPI b_pow = bi_powi(b, n-t);
	MPI loop_check = bi_mul(y, b_pow);
	while(bi_ge(r, loop_check)){
		q->data[n-t]++;

		MPI tmp = bi_sub(r, loop_check);
		bi_copy(tmp, r);
		bi_free(tmp);
	}

	for(int i = n; i > t; i--){
		if (r->data[i] == y->data[t]) {
			q->data[i-t-1] = 0xFFFFFFFFu;
		} else {
			int tmp = (0x100000000ul * (unsigned long)(r->data[i]) + (unsigned long)(r->data[i-1])) 
				/ (unsigned long)(y->data[i]);
			q->data[i-t-1] = (unsigned int)(tmp);
		}

		MPI inner_counter = bi_init_like(r);
		bi_set(inner_counter, 0u);
		inner_counter->data[0] = r->data[i-2];
		inner_counter->data[1] = r->data[i-1];
		inner_counter->data[2] = r->data[i-0];

		MPI loop_tmp = bi_init_like(r);
		bi_set(loop_tmp, 0u);
		loop_tmp->data[0] = y->data[t-1];
		loop_tmp->data[1] = y->data[t-0];

		MPI loop_tmp2 = bi_init_like(r);
		bi_set(loop_tmp2, 0u);
		loop_tmp2->data[0] = q->data[i-t-1];


		MPI inner_loop_check = bi_mul(loop_tmp, loop_tmp2);

		while(bi_gt(inner_loop_check, inner_counter)){
			q->data[i-t-1]--;

			loop_tmp2->data[0] = q->data[i-t-1];
			bi_free(inner_loop_check);
			inner_loop_check = bi_mul(loop_tmp, loop_tmp2);
		}

		MPI tmp1 = bi_shift_left(y, i-t-1);
		MPI tmp2 = bi_mul(loop_tmp2, tmp1);

		if(bi_gt(tmp2, r)){
			MPI tmp4 = bi_add(r, tmp1);
			bi_copy(tmp4, r);
			bi_free(tmp4);

			q->data[i-t-1]--;
		}

		MPI tmp3 = bi_sub(r, tmp2);
		bi_copy(tmp3, r);

		bi_free(inner_counter);
		bi_free(loop_tmp);
		bi_free(loop_tmp2);
		bi_free(inner_loop_check);
		bi_free(tmp1);
		bi_free(tmp2);
		bi_free(tmp3);
	}

	return r;
	*/
}

MPI bi_eucl_div(MPI x, MPI y)
{
	//TODO: handle div by zero
	MPI q = bi_init_like(x);
	MPI r = bi_init_like(x);

	bi_set(q, 0u);
	bi_set(r, 0u);

	int div_bits = 32u * x->words;
	for(int i = div_bits - 1; i >= 0; i--){
		// Left-shift R by 1 bit
		MPI tmp = bi_shift_left(r, 1u);
		bi_copy(tmp, r);
		bi_free(tmp);

		// Set the least-significant bit of R equal to bit i of the numerator
		unsigned int curr_word = i / 32u;
		unsigned int curr_word_pos = i % 32u;
		unsigned int res = x->data[curr_word] & (1u << curr_word_pos);
		r->data[0] |= res >> curr_word_pos;

		if(bi_ge(r, y)){
			// r := r - y
			tmp = bi_sub(r, y);
			bi_copy(tmp, r);
			bi_free(tmp);

			q->data[curr_word] |= 1u << curr_word_pos;
		}
	}

	bi_free(r);	
	bi_squeeze(q);
	return q;
}

void bi_printf(MPI x){
	printf("0x");
	for(int i = x->words - 1; i > 0; i--){
		printf("%08x_", x->data[i]);
	}
	printf("%08x", x->data[0]);

}
void bi_inc(MPI x)
{
	int i = 0;
	while(x->data[i] == 0xFFFFFFFF){
		x->data[i] = 0u;
		i++;
	}

	x->data[i]++;

	bi_squeeze(x);
}

void bi_dec(MPI x)
{
	int i = 0;
	while(x->data[i] == 0u){
		x->data[i] = 0xFFFFFFFF;
		i++;
	}

	x->data[i]--;

	bi_squeeze(x);
}
MPI bi_and(MPI a, MPI b)
{
	MPI res = bi_init(max(a->words, b->words));

	for(int i = 0; i < min(a->words, b->words); i++){
		res->data[i] = a->data[i] & b->data[i];
	}

	bi_squeeze(res);

	return res;
}
MPI bi_or(MPI a, MPI b)
{
	MPI res = bi_init(max(a->words, b->words));

	for(int i = 0; i < min(a->words, b->words); i++){
		res->data[i] = a->data[i] | b->data[i];
	}

	if(a->words > b->words){
		for(int i = b->words; i < a->words; i++){
			res->data[i] = a->data[i];
		}
	} else if (a->words < b->words){
		for(int i = a->words; i < b->words; i++){
			res->data[i] = b->data[i];
		}
	}

	bi_squeeze(res);

	return res;
}
MPI bi_xor(MPI a, MPI b)
{
	MPI res = bi_init(max(a->words, b->words));

	for(int i = 0; i < min(a->words, b->words); i++){
		res->data[i] = a->data[i] ^ b->data[i];
	}

	if(a->words > b->words){
		for(int i = b->words; i < a->words; i++){
			res->data[i] = a->data[i];
		}
	} else if (a->words < b->words){
		for(int i = a->words; i < b->words; i++){
			res->data[i] = b->data[i];
		}
	}

	bi_squeeze(res);

	return res;
}
MPI bi_not(MPI a)
{
	MPI res = bi_init_like(a);

	for(int i = 0; i < a->words; i++){
		res->data[i] = ~a->data[i];
	}


	bi_squeeze(res);
	return res;
}

MPI bi_shift_left(MPI a, unsigned int n)
{
	MPI res = bi_init_like(a);

	int offset_words = n / 32;
	int offset_mod = n % 32;

	for(int i = offset_words; i < res->words; i++){
		res->data[i] = (a->data[i - offset_words] << offset_mod);
		
		if(i > 0)
			res->data[i] |= (a->data[i - offset_words - 1] >> (32u - offset_mod));
	}

	bi_squeeze(res);
	return res;
}

MPI bi_shift_right(MPI a, unsigned int n)
{
	MPI res = bi_init_like(a);

	int offset_words = n / 32;
	int offset_mod = n % 32;

	for(int i = 0; i < res->words - offset_words; i++){
		res->data[i] = (a->data[i - offset_words] >> offset_mod);
		
		if(i < res->words - 1)
			res->data[i] |= (a->data[i - offset_words + 1] << (32u - offset_mod));
	}

	bi_squeeze(res);
	return res;
}


// This is a very simple modular exponentiation algorithm. There are
// faster, more efficient algorithms that can be implemented later.
MPI bi_mod_exp(MPI x, MPI exp,
		MPI mod)
{
	MPI res = bi_init_like(x);

	if(bi_eq_val(mod, 1u)){
		bi_set(res, 0u);
		return res;
	}

	bi_set(res, 1u);

	// both the mod and the exp are bigints, which means
	// we have to use loop methods that support them

	MPI loop_counter = bi_init_like(x);
	bi_set(loop_counter, 0u);

	MPI tmp;
	while(bi_lt(loop_counter, exp)){
		// res := (res * base) % mod

		tmp = bi_mul(res, x);
		bi_copy(tmp, res);
		bi_free(tmp);

		tmp = bi_mod(res, mod);
		bi_copy(tmp, res);
		bi_free(tmp);

		bi_inc(loop_counter);
	}

	return res;
}

bool bi_lt(MPI a, MPI b)
{
	if (a->words > b->words) {
		for(int i = b->words; i < a->words; i++){
			if(a->data[i] > 0u){
				return false;
			}
		}
	} else if(b->words > a->words) {
		for(int i = a->words; i < b->words; i++){
			if(b->data[i] > 0u){
				return true;
			}
		}
	}
	if(a->words != b->words){
		// Honestly, not sure what to do here. I think for now I'll
		// just return false
		return false;
	}

	for(int i = a->words - 1; i >= 0; i--){
		if(a->data[i] > b->data[i]){
			return false;
		} else if (a->data[i] < b->data[i]){
			return true;
		}
	}

	// a and b are equal
	return false;
}

bool bi_le(MPI a, MPI b)
{
	if (a->words > b->words) {
		for(int i = b->words; i < a->words; i++){
			if(a->data[i] > 0u){
				return false;
			}
		}
	} else if(b->words > a->words) {
		for(int i = a->words; i < b->words; i++){
			if(b->data[i] > 0u){
				return true;
			}
		}
	}

	if(a->words != b->words){
		// Honestly, not sure what to do here. I think for now I'll
		// just return false
		return false;
	}

	for(int i = a->words - 1; i > 0; i--){
		if(a->data[i] > b->data[i]){
			return false;
		} else if (a->data[i] < b->data[i]){
			return true;
		}
	}

	// a and b are equal
	return true;
}

bool bi_eq(MPI a, MPI b)
{
	for(int i = 0; i < min(a->words, b->words); i++){
		if(a->data[i] != b->data[i])
			return false;
	}

	if(a->words > b->words){
		for(int i = b->words; i < a->words; i++){
			if(a->data[i] != 0u){
				return false;
			}
		}
	} else if (b->words > a->words) {
		for(int i = a->words; i < b->words; i++){
			if(b->data[i] != 0u){
				return false;
			}
		}
	}

	return true;
}

bool bi_eq_val(MPI a, unsigned int b)
{
	for(int i = 1; i < a->words - 1; i++){
		if(a->data[i] != 0u)
			return false;
	}

	return a->data[0] == b;
}

bool bi_gt(MPI a, MPI b){
	if (a->words > b->words) {
		for(int i = b->words; i < a->words; i++){
			if(a->data[i] > 0u){
				return true;
			}
		}
	} else if(b->words > a->words) {
		for(int i = a->words; i < b->words; i++){
			if(b->data[i] > 0u){
				return false;
			}
		}
	}

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
bool bi_ge(MPI a, MPI b){
	if (a->words > b->words) {
		for(int i = b->words; i < a->words; i++){
			if(a->data[i] > 0u){
				return true;
			}
		}
	} else if(b->words > a->words) {
		for(int i = a->words; i < b->words; i++){
			if(b->data[i] > 0u){
				return false;
			}
		}
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


MPI bi_concat(MPI a, MPI b)
{
	MPI res;
	res->words = a->words + b->words;

	res->data = malloc(res->words * sizeof(unsigned int));
	memcpy(res->data, a->data, a->words * sizeof(unsigned int));
	memcpy(&(res->data[a->words]), b->data, b->words * sizeof(unsigned int));

	return res;
}

MPI bi_init_and_copy(MPI src)
{
	MPI res= bi_init_like(src);

	bi_copy(src, res);

	return res;
}

bool bi_even(MPI a)
{
	return !(a->data[0] & 1u);
}

MPI pad(MPI x, int n){
	MPI res = bi_init(n);
	res->words = x->words + n;

	for(int i = 0; i < x->words; i++){
		res->data[i] = x->data[i];
	}

	return res;
}

void bi_squeeze(MPI x)
{
	int trim_idx = 0;
	for(int i = x->words - 1; i > 0; i--) {
		if (x->data[i] > 0u) {
			trim_idx = i;
			break;
		}
	}

	if(trim_idx < x->words - 1) {
		int new_words = trim_idx + 1;
		x->data = realloc(x->data, new_words * sizeof(unsigned int));
		x->words = new_words;
	}
}
