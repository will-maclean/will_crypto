#include "bigint.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline bi_result_t bi_result_make(MPI x, bi_result_code_t code)
{
    bi_result_t res = {x, code};
    return res;
}

static inline bi_result_t bi_result_error(bi_result_code_t code)
{
    return bi_result_make(NULL, code);
}

int assert_same_shape(MPI a, MPI b) { return a->words == b->words; }

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

bi_result_t __bi_init(int words)
{
    MPI x = malloc(sizeof(struct bigint));

    if (x == NULL) {
        return bi_result_error(BI_MEM_ERR);
    }

    if (words == 0) {
        printf("WARNING: bi_init called with words=0. You probably don't want "
               "to do this...");
    }

    x->words = words;
    x->data = NULL;

    if (words > 0) {
        x->data = calloc((size_t)words, sizeof(uint32_t));
        if (x->data == NULL) {
            free(x);
            return bi_result_error(BI_MEM_ERR);
        }
    }

    return bi_result_make(x, BI_OK);
}

MPI bi_init(int words){
    bi_result_t res = __bi_init(words);
    if (res.code != BI_OK) {
        bi_free(res.x);
        return NULL;
    }
    return res.x;
}

MPI bi_init_like(MPI like)
{
    return bi_init(like->words);
}

bi_result_code_t __bi_copy(MPI src, MPI target)
{
    if (src == target) {
        return BI_OK;
    }

    uint32_t *new_data = NULL;

    if (src->words > 0) {
        size_t bytes = (size_t)src->words * sizeof(uint32_t);
        new_data = malloc(bytes);
        if (new_data == NULL) {
            return BI_MEM_ERR;
        }
        memcpy(new_data, src->data, bytes);
    }

    free(target->data);
    target->data = new_data;
    target->words = src->words;

    return BI_OK;
}

void bi_copy(MPI src, MPI target)
{
    bi_result_code_t code = __bi_copy(src, target);
    if (code != BI_OK) {
        fprintf(stderr, "FATAL: bi_copy failed with code %d\n", code);
        exit(1);
    }
}

void bi_free(MPI x)
{
    free(x->data);
    free(x);
}

bi_result_code_t __bi_set(MPI a, uint32_t val)
{
    uint32_t *data = realloc(a->data, sizeof(uint32_t));

    if (data == NULL) {
        return BI_MEM_ERR;
    }

    a->data = data;
    a->data[0] = val;
    a->words = 1;

    return BI_OK;
}

void bi_set(MPI a, uint32_t val) {
    free(a->data);
    a->data = malloc(sizeof(uint32_t));
    a->data[0] = val;
    a->words = 1;
}

MPI bi_add(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words) + 1);

    uint32_t carry = 0;
    unsigned long sum;
    for (int i = 0; i < min(a->words, b->words); i++) {
        sum = (unsigned long)(a->data[i]) + (unsigned long)b->data[i] +
              res->data[i] + carry;
        res->data[i] = (uint32_t)(sum & 0xFFFFFFFF);

        /*
        printf("step: %d. a->data[i]=%u, b->data[i]=%u, res->data[i]=%u,
        carry=%u. sum=%lu\nres=", i, a->data[i], b->data[i], (*res)->data[i],
        carry, sum); bi_printf(*res); printf("\n");
        */

        carry = (uint32_t)(sum >> 32);

        if (carry && i > 0) {
            res->data[i + 1] += carry;
        }
    }

    if (a->words > b->words) {
        res->data[b->words] = carry + a->data[b->words];
        for (int i = b->words + 1; i < a->words; i++) {
            res->data[i] = a->data[i];
        }
    } else if (b->words > a->words) {
        res->data[a->words] = carry + b->data[a->words];
        for (int i = a->words + 1; i < b->words; i++) {
            res->data[i] = b->data[i];
        }
    } else {
        res->data[res->words - 1] = carry;
    }

    bi_squeeze(res);
    return res;
}

MPI bi_sub(MPI a, MPI b) {

    // we're working with uint32_ts, so if b is greater than a, we'll
    // set the result to 0 and return early
    if (!bi_ge(a, b)) {
        MPI res = bi_init_like(a);
        bi_set(res, 0u);
        return res;
    }

    // There may be scenarios where, even though a >= b,
    // b.words > a.words. So, to catch this, we'll pad
    // a to the size of b
    MPI tmp = bi_init_and_copy(a);
    MPI a_copy = pad(tmp, b->words > a->words ? b->words - a->words : 0);
    MPI res = bi_init_like(a_copy);

    // invariant: a >= b

    int n = b->words;

    uint64_t base = 1LL << 32;
    uint64_t carry = base;
    for (int i = 0; i < n; i++) {

        carry = base - 1 + a_copy->data[i] - b->data[i] + carry / base;

        res->data[i] = carry % base;
    }

    bi_free(tmp);
    bi_free(a_copy);

    return res;
}

MPI _bi_sub(MPI a, MPI b) {
    MPI res = bi_init_like(a);
    MPI a_copy = bi_init_like(a);
    bi_copy(a, a_copy);

    // we're working with uint32_ts, so if b is greater than a, we'll
    // set the result to 0 and return early
    if (!bi_ge(a, b)) {
        bi_set(res, 0u);
        return res;
    }

    // sanity check...
    if (a->words == 0 || b->words == 0) {
        printf(
            "WARNIGN: trying to do subtraction with 0-length MPIs. a.words=%d, "
            "b.words=%d",
            a->words, b->words);
    }

    // TODO: future Will -> this can't be the best way to do this. Surely
    // we'll need to revisit this and make it smarter.

    uint64_t sub_from;
    for (int i = 0; i < b->words; i++) {
        if (a_copy->data[i] >= b->data[i]) {
            res->data[i] = a_copy->data[i] - b->data[i];
        } else {
            // we need to go and get that bump
            int j = i + 1;

            // go to the left until we find something greater than
            // 0.
            while (j >= 0 && a_copy->data[j] == 0u)
                j++;

            // we found something greater then zero. We'll decrement
            // it and then carry the value back to the right again

            a_copy->data[j]--;

            // now move back to the right again, setting all the
            // zeros we find along the way to the max value (we know
            // they're zeros currently)
            j--;
            while (j != i) {
                a_copy->data[j] = 0xFFFFFFFF;
                j--;
            }

            // we can now compute the subtraction for the ith
            // place

            sub_from = (unsigned long)a_copy->data[i];
            sub_from += 1ul << 63;
            sub_from -= (unsigned long)b->data[i];
            sub_from &= 0xFFFFFFFF;
            res->data[i] = (uint32_t)sub_from;
        }
        /*
        printf("step: %d. a->data[i]=%u, a_copy->data[i]=%u, b->data[i]=%u,
        res->data[i]=%u\nres=", i, a->data[i], a_copy->data[i], b->data[i],
        (*res)->data[i]); bi_printf(*res); printf("\n");
        */
    }

    for (int i = b->words; i < res->words; i++) {
        res->data[i] = a->data[i];
    }

    bi_free(a_copy);

    bi_squeeze(res);
    return res;
}

MPI bi_mul_imm(MPI a, uint32_t x) {
    MPI res = bi_init(2 * a->words);
    uint64_t base = 1LL << 32;
    uint64_t carry = 0;

    for (int i = 0; i < a->words; i++) {
        res->data[i] = (a->data[i] * x + carry) % base;
        carry = (a->data[i] * x + carry) / base;
    }

    return res;
}

MPI bi_mul(MPI a, MPI b) {
    MPI res = bi_init(2 * max(a->words, b->words));

    uint64_t carry;
    uint64_t prod;
    for (int i = 0; i < a->words; i++) {
        carry = 0;
        for (int j = 0; j < b->words; j++) {
            prod = (uint64_t)(a->data[i]) * (uint64_t)b->data[j] +
                   res->data[i + j] + carry;
            // printf("i=%d, j=%d, prod=%lu\n", i, j, prod);
            res->data[i + j] = (uint32_t)(prod & 0xFFFFFFFF);
            /*
            printf("res=");
            bi_printf(*res);
            printf("\n");
            */
            carry = (uint32_t)(prod >> 32);
        }

        if (carry) {
            res->data[i + 1] += carry;
        }
    }

    bi_squeeze(res);

    return res;
}

MPI bi_pow_imm(MPI b, uint32_t p) {
    // Basic exponentiation algorithm, plenty of faster ones
    // out there if required. I don't think this can take
    // more then log2(p) iterations, which is pretty good,
    // so we should be safe for a while.

    MPI a = bi_init(2 * b->words);
    bi_set(a, 1u);
    MPI s = bi_init(2 * b->words);

    for (int i = 0; i < b->words; i++) {
        s->data[i] = b->data[i];
    }

    while (p != 0u) {
        if (p % 2 == 1) {
            MPI tmp = bi_mul(a, s);
            bi_copy(tmp, a);
            bi_free(tmp);
        }

        p /= 2;

        if (p != 0) {
            MPI tmp = bi_mul(s, s);
            bi_copy(tmp, s);
            bi_free(tmp);
        }
    }

    bi_free(s);

    return a;
}

// x % y
MPI bi_mod(MPI x, MPI y) {
    // TODO: handle div by zero
    if (bi_gt(y, x)) {
        MPI res = bi_init_and_copy(x);
        return res;
    }

    int y_orig_words;
    bool y_was_squeezed = false;
    if (x->words > y->words) {
        y_was_squeezed = true;
        y_orig_words = y->words;
        MPI tmp = pad(y, x->words - y->words);
        bi_copy(tmp, y);
        bi_free(tmp);
    }
    MPI r = bi_init_like(x);
    bi_set(r, 0u);

    int div_bits = 32u * x->words;
    for (int i = div_bits - 1; i >= 0; i--) {
        // Left-shift R by 1 bit
        MPI tmp = bi_shift_left(r, 1u);
        bi_copy(tmp, r);
        bi_free(tmp);

        // Set the least-significant bit of R equal to bit i of the numerator
        uint32_t curr_word = i / 32u;
        uint32_t curr_word_pos = i % 32u;
        uint32_t res = x->data[curr_word] & (1u << curr_word_pos);
        r->data[0] |= res >> curr_word_pos;

        if (bi_ge(r, y)) {
            // r := r - y
            tmp = bi_sub(r, y);
            bi_copy(tmp, r);
            bi_free(tmp);
        }
    }

    if (y_was_squeezed) {
        bi_squeeze(y);

        if (y->words < y_orig_words) {
            MPI tmp = pad(y, y_orig_words - y->words);
            bi_copy(tmp, y);
            bi_free(tmp);
        }
    }

    bi_squeeze(r);
    return r;
}

MPI bi_eucl_div(MPI x, MPI y) {
    // TODO: handle div by zero
    if (bi_gt(y, x)) {
        MPI res = bi_init(1);
        bi_set(res, 0u);
        return res;
    }

    int y_orig_words;
    bool y_was_squeezed = false;
    if (x->words > y->words) {
        y_was_squeezed = true;
        y_orig_words = y->words;
        MPI tmp = pad(y, x->words - y->words);
        bi_copy(tmp, y);
        bi_free(tmp);
    }

    MPI q = bi_init_like(x);
    MPI r = bi_init_like(x);

    bi_set(q, 0u);
    bi_set(r, 0u);

    int div_bits = 32u * x->words;
    for (int i = div_bits - 1; i >= 0; i--) {
        // Left-shift R by 1 bit
        MPI tmp = bi_shift_left(r, 1u);
        bi_copy(tmp, r);
        bi_free(tmp);

        // Set the least-significant bit of R equal to bit i of the numerator
        uint32_t curr_word = i / 32u;
        uint32_t curr_word_pos = i % 32u;
        uint32_t res = x->data[curr_word] & (1u << curr_word_pos);
        r->data[0] |= res >> curr_word_pos;

        if (bi_ge(r, y)) {
            // r := r - y
            tmp = bi_sub(r, y);
            bi_copy(tmp, r);
            bi_free(tmp);

            q->data[curr_word] |= 1u << curr_word_pos;
        }
    }

    if (y_was_squeezed) {
        bi_squeeze(y);

        if (y->words < y_orig_words) {
            MPI tmp = pad(y, y_orig_words - y->words);
            bi_copy(tmp, y);
            bi_free(tmp);
        }
    }

    bi_free(r);
    bi_squeeze(q);
    return q;
}

void bi_printf(MPI x) {
    printf("0x");
    for (int i = x->words - 1; i > 0; i--) {
        printf("%08x_", x->data[i]);
    }
    printf("%08x", x->data[0]);
}

void bi_inc(MPI x) {
    int i = 0;
    while (x->data[i] == 0xFFFFFFFF && i < x->words) {
        x->data[i] = 0u;
        i++;
    }

    if (i == x->words) {
        MPI tmp = pad(x, 1);
        bi_copy(tmp, x);
        bi_free(tmp);

        x->data[x->words - 1] = 1u;
    } else {
        x->data[i]++;
    }

    bi_squeeze(x);
}

void bi_dec(MPI x) {
    int i = 0;
    while (x->data[i] == 0u) {
        x->data[i] = 0xFFFFFFFF;
        i++;
    }

    x->data[i]--;

    bi_squeeze(x);
}
MPI bi_and(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words));

    for (int i = 0; i < min(a->words, b->words); i++) {
        res->data[i] = a->data[i] & b->data[i];
    }

    bi_squeeze(res);

    return res;
}
MPI bi_or(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words));

    for (int i = 0; i < min(a->words, b->words); i++) {
        res->data[i] = a->data[i] | b->data[i];
    }

    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            res->data[i] = a->data[i];
        }
    } else if (a->words < b->words) {
        for (int i = a->words; i < b->words; i++) {
            res->data[i] = b->data[i];
        }
    }

    bi_squeeze(res);

    return res;
}
MPI bi_xor(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words));

    for (int i = 0; i < min(a->words, b->words); i++) {
        res->data[i] = a->data[i] ^ b->data[i];
    }

    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            res->data[i] = a->data[i];
        }
    } else if (a->words < b->words) {
        for (int i = a->words; i < b->words; i++) {
            res->data[i] = b->data[i];
        }
    }

    bi_squeeze(res);

    return res;
}
MPI bi_not(MPI a) {
    MPI res = bi_init_like(a);

    for (int i = 0; i < a->words; i++) {
        res->data[i] = ~a->data[i];
    }

    bi_squeeze(res);
    return res;
}

MPI bi_shift_left(MPI a, uint32_t n) {
    MPI res = bi_init_like(a);

    int offset_words = n / 32;
    int offset_mod = n % 32;

    for (int i = offset_words; i < res->words; i++) {
        res->data[i] = (a->data[i - offset_words] << offset_mod);

        if (i > 0)
            res->data[i] |=
                (a->data[i - offset_words - 1] >> (32u - offset_mod));
    }

    bi_squeeze(res);
    return res;
}

MPI bi_shift_right(MPI a, uint32_t n) {
    MPI res = bi_init_like(a);

    int offset_words = n / 32;
    int offset_mod = n % 32;

    for (int i = 0; i < res->words - offset_words; i++) {
        res->data[i] = (a->data[i - offset_words] >> offset_mod);

        if (i < res->words - 1)
            res->data[i] |=
                (a->data[i - offset_words + 1] << (32u - offset_mod));
    }

    bi_squeeze(res);
    return res;
}

MPI bi_mod_exp(MPI a, MPI b, MPI n) {
    if (bi_eq_val(b, 0u)) {
        MPI res = bi_init(1u);
        bi_set(res, 1u);
        return res;
    }

    if (bi_even(b)) {
        MPI tmp_b = bi_shift_right(b, 1u);
        MPI x = bi_mod_exp(a, tmp_b, n);
        bi_free(tmp_b);

        MPI tmp_x = bi_mod(x, n);
        bi_copy(tmp_x, x);
        bi_free(tmp_x);

        tmp_x = bi_mul(x, x);
        bi_copy(tmp_x, x);
        bi_free(tmp_x);

        tmp_x = bi_mod(x, n);
        bi_free(x);

        return tmp_x;
    }

    bi_dec(b);
    MPI tmp = bi_mod_exp(a, b, n);
    MPI tmp2 = bi_mul(a, tmp);
    MPI tmp3 = bi_mod(tmp2, n);

    bi_free(tmp);
    bi_free(tmp2);

    bi_inc(b);
    return tmp3;
}

bool bi_lt(MPI a, MPI b) {
    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return false;
            }
        }
    } else if (b->words > a->words) {
        for (int i = a->words; i < b->words; i++) {
            if (b->data[i] > 0u) {
                return true;
            }
        }
    }
    if (a->words != b->words) {
        // Honestly, not sure what to do here. I think for now I'll
        // just return false
        return false;
    }

    for (int i = a->words - 1; i >= 0; i--) {
        if (a->data[i] > b->data[i]) {
            return false;
        } else if (a->data[i] < b->data[i]) {
            return true;
        }
    }

    // a and b are equal
    return false;
}

bool bi_le(MPI a, MPI b) {
    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return false;
            }
        }
    } else if (b->words > a->words) {
        for (int i = a->words; i < b->words; i++) {
            if (b->data[i] > 0u) {
                return true;
            }
        }
    }

    if (a->words != b->words) {
        // Honestly, not sure what to do here. I think for now I'll
        // just return false
        return false;
    }

    for (int i = a->words - 1; i > 0; i--) {
        if (a->data[i] > b->data[i]) {
            return false;
        } else if (a->data[i] < b->data[i]) {
            return true;
        }
    }

    // a and b are equal
    return true;
}

bool bi_eq(MPI a, MPI b) {
    // TODO: a and b can be numerically equicalent, but
    //  different word sizes, and fail this equality test.
    //  This probably isn't what we want??
    for (int i = 0; i < min(a->words, b->words); i++) {
        if (a->data[i] != b->data[i])
            return false;
    }

    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            if (a->data[i] != 0u) {
                return false;
            }
        }
    } else if (b->words > a->words) {
        for (int i = a->words; i < b->words; i++) {
            if (b->data[i] != 0u) {
                return false;
            }
        }
    }

    return true;
}

bool bi_eq_val(MPI a, uint32_t b) {
    for (int i = 1; i < a->words - 1; i++) {
        if (a->data[i] != 0u)
            return false;
    }

    return a->data[0] == b;
}

bool bi_gt(MPI a, MPI b) {
    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return true;
            }
        }
    } else if (b->words > a->words) {
        for (int i = a->words; i < b->words; i++) {
            if (b->data[i] > 0u) {
                return false;
            }
        }
    }

    if (a->words != b->words) {
        // Honestly, not sure what to do here. I think for now I'll
        // just return false
        return false;
    }

    for (int i = 0; i < a->words; i++) {
        if (a->data[i] > b->data[i]) {
            return true;
        } else if (a->data[i] < b->data[i]) {
            return false;
        }
    }

    // a and b are equal
    return false;
}
bool bi_ge(MPI a, MPI b) {
    if (a->words > b->words) {
        for (int i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return true;
            }
        }
    } else if (b->words > a->words) {
        for (int i = a->words; i < b->words; i++) {
            if (b->data[i] > 0u) {
                return false;
            }
        }
    }

    for (int i = 0; i < a->words; i++) {
        if (a->data[i] > b->data[i]) {
            return true;
        } else if (a->data[i] < b->data[i]) {
            return false;
        }
    }

    // a and b are equal
    return true;
}

MPI bi_concat(MPI a, MPI b) {
    MPI res = bi_init(a->words + b->words);

    res->data = malloc(res->words * sizeof(uint32_t));
    memcpy(res->data, a->data, a->words * sizeof(uint32_t));
    memcpy(&(res->data[a->words]), b->data, b->words * sizeof(uint32_t));

    return res;
}

MPI bi_init_and_copy(MPI src) {
    MPI res = bi_init_like(src);

    bi_copy(src, res);

    return res;
}

bool bi_even(MPI a) { return !(a->data[0] & 1u); }

MPI pad(MPI x, int n) {
    MPI res = bi_init(x->words + n);
    res->words = x->words + n;

    for (int i = 0; i < x->words; i++) {
        res->data[i] = x->data[i];
    }

    return res;
}

void bi_squeeze(MPI x) {
    int squeeze_idx = 0;
    bool squeeze_needed = false;
    for (int i = x->words - 1; i > 0; i--) {
        if (x->data[i] == 0u) {
            squeeze_needed = true;
        }
        if (x->data[i] != 0 && squeeze_needed) {
            squeeze_idx = i;
        }
    }

    int squeezed_words = squeeze_idx + 1;
    if (squeezed_words < x->words) {
        x->data = realloc(x->data, squeezed_words * sizeof(uint32_t));
        x->words = squeezed_words;
    }
}
