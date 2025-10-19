#include <bigint/bigint.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    BI_OK,
    BI_MEM_ERR,
    BI_DIV_ZERO,
    BI_BAD_OPERANDS,
} __bi_result_code_t;

typedef struct {
    MPI x;
    __bi_result_code_t code;
} __bi_result_t;

static inline __bi_result_t bi_result_make(MPI x, __bi_result_code_t code) {
    __bi_result_t res = {x, code};
    return res;
}

static inline __bi_result_t bi_result_error(__bi_result_code_t code) {
    return bi_result_make(NULL, code);
}

__bi_result_t __knuth_d(MPI u, MPI v, bool return_quotient);
__bi_result_t __bi_div_imm(MPI a, uint32_t b, bool return_quotient);

int assert_same_shape(MPI a, MPI b) { return a->words == b->words; }

uint32_t min(uint32_t a, uint32_t b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

uint32_t max(uint32_t a, uint32_t b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

__bi_result_t __bi_init(uint32_t words) {
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

MPI bi_init(uint32_t words) {
    __bi_result_t res = __bi_init(words);
    if (res.code != BI_OK) {
        bi_free(res.x);
        return NULL;
    }
    return res.x;
}

MPI bi_init_like(MPI like) { return bi_init(like->words); }

__bi_result_code_t __bi_copy(MPI src, MPI target) {
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

void bi_copy(MPI src, MPI target) {
    __bi_result_code_t code = __bi_copy(src, target);
    if (code != BI_OK) {
        fprintf(stderr, "FATAL: bi_copy failed with code %d\n", code);
        exit(1);
    }
}

void bi_free(MPI x) {
    free(x->data);
    free(x);
}

__bi_result_code_t __bi_set(MPI a, uint32_t val) {
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
    for (uint32_t i = 0; i < min(a->words, b->words); i++) {
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
        for (uint32_t i = b->words + 1; i < a->words; i++) {
            res->data[i] = a->data[i];
        }
    } else if (b->words > a->words) {
        res->data[a->words] = carry + b->data[a->words];
        for (uint32_t i = a->words + 1; i < b->words; i++) {
            res->data[i] = b->data[i];
        }
    } else {
        res->data[res->words - 1] = carry;
    }

    bi_squeeze(res);
    return res;
}

void bi_add_in_place(MPI a, MPI b) {
    bi_squeeze(a);
    uint32_t carry = 0;
    unsigned long sum;
    for (uint32_t i = 0; i < min(a->words, b->words); i++) {
        sum = (unsigned long)(a->data[i]) + (unsigned long)b->data[i] + carry;
        a->data[i] = (uint32_t)(sum & 0xFFFFFFFF);

        carry = (uint32_t)(sum >> 32);

        if (carry && i > 0) {
            a->data[i + 1] += carry;
        }
    }

    if (carry) {
        bi_pad_words(a, 1);
        a->data[a->words - 1] = carry;
    }
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
    MPI a_copy =
        bi_pad_words(tmp, b->words > a->words ? b->words - a->words : 0);
    MPI res = bi_init_like(a_copy);

    // invariant: a >= b

    uint32_t n = b->words;

    uint64_t base = 1LL << 32;
    uint64_t carry = base;
    for (uint32_t i = 0; i < n; i++) {

        carry = base - 1 + a_copy->data[i] - b->data[i] + carry / base;

        res->data[i] = carry % base;
    }

    bi_free(tmp);
    bi_free(a_copy);

    return res;
}

// a -= b
void bi_sub_in_place(MPI a, MPI b) {
    // TODO: actually implement an in place subtraction
    MPI res = bi_sub(a, b);
    bi_free(a);
    a->data = res->data;
    a->words = res->words;
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
    for (uint32_t i = 0; i < b->words; i++) {
        if (a_copy->data[i] >= b->data[i]) {
            res->data[i] = a_copy->data[i] - b->data[i];
        } else {
            // we need to go and get that bump
            uint32_t j = i + 1;

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

    for (uint32_t i = b->words; i < res->words; i++) {
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

    for (uint32_t i = 0; i < a->words; i++) {
        res->data[i] = (a->data[i] * x + carry) % base;
        carry = (a->data[i] * x + carry) / base;
    }

    return res;
}

MPI bi_mul(MPI a, MPI b) {
    // linear convolution
    // https://www.hvks.com/Numerical/Downloads/HVE%20The%20Math%20behind%20arbitrary%20precision.pdf
    // page 11/12

    uint32_t n = a->words;
    uint32_t m = b->words;
    MPI res = bi_init(n + m);

    for (uint32_t i = 0; i < m; i++) {
        uint64_t carry = 0;
        for (uint32_t j = 0; j < n; j++) {
            uint64_t tmp = (uint64_t)a->data[j] * (uint64_t)b->data[i];
            uint32_t tmp_low = (uint32_t)tmp;
            uint32_t tmp_hi = (uint32_t)(tmp >> 32);
            uint32_t s1 = tmp_low + res->data[i + j];
            uint32_t c1 = s1 < tmp_low;
            uint32_t carry_low = (uint32_t)carry;
            uint32_t s2 = s1 + carry_low;
            uint32_t c2 = s2 < carry_low;
            res->data[i + j] = s2;
            carry = (uint64_t)tmp_hi + c1 + c2 + (carry >> 32);
        }

        uint32_t k = i + n;
        while (carry) {
            uint32_t carry_low = (uint32_t)carry;
            uint32_t old = res->data[k];
            res->data[k] += carry_low;
            uint32_t spill = res->data[k] < old;
            carry = (carry >> 32) + spill;
            k++;
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
    //
    // https://www.hvks.com/Numerical/Downloads/HVE%20The%20Math%20behind%20arbitrary%20precision.pdf
    // page 30

    MPI r = bi_init(2 * b->words);
    r->data[0] = 1;

    MPI b_copy = bi_init_and_copy(b);

    while (p != 0u) {
        if (p % 2 == 1) {
            MPI tmp = bi_mul(r, b_copy);
            bi_copy(tmp, r);
            bi_free(tmp);
        }

        p /= 2;

        MPI tmp = bi_mul(b_copy, b_copy);
        bi_free(b_copy);
        b_copy = bi_init_and_copy(tmp);
        bi_free(tmp);
    }

    bi_free(b_copy);

    return r;
}

// x % y
MPI bi_mod(MPI x, MPI y) {

    __bi_result_t res = __knuth_d(x, y, false);

    if (res.code != BI_OK) {
        printf("bi_mod failed with error code: %d\n", res.code);
        exit(1);
    }

    return res.x;
}
MPI bi_mod_old(MPI x, MPI y) {
    // TODO: handle div by zero
    if (bi_gt(y, x)) {
        MPI res = bi_init_and_copy(x);
        return res;
    }

    uint32_t y_orig_words;
    bool y_was_squeezed = false;
    if (x->words > y->words) {
        y_was_squeezed = true;
        y_orig_words = y->words;
        MPI tmp = bi_pad_words(y, x->words - y->words);
        bi_copy(tmp, y);
        bi_free(tmp);
    }
    MPI r = bi_init_like(x);
    bi_set(r, 0u);

    uint32_t div_bits = 32u * x->words;
    for (int32_t i = div_bits - 1; i >= 0; i--) {
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
            MPI tmp = bi_pad_words(y, y_orig_words - y->words);
            bi_copy(tmp, y);
            bi_free(tmp);
        }
    }

    bi_squeeze(r);
    return r;
}

uint32_t leading_zeros(uint32_t x) {
    // gcc has a builtin helper for this
    return __builtin_clz(x);
}

// u / v
__bi_result_t __knuth_d(MPI u, MPI v, bool return_quotient) {
    bi_squeeze(u);
    bi_squeeze(v);

    if (bi_eq_val(v, 0)) {
        return bi_result_error(BI_DIV_ZERO);
    }

    if (bi_eq(u, v)) {
        if (return_quotient) {
            MPI res = bi_init(1);
            bi_set(res, 1);
            return bi_result_make(res, BI_OK);
        } else {
            MPI res = bi_init(1);
            bi_set(res, 0);
            return bi_result_make(res, BI_OK);
        }
    }

    if (bi_gt(v, u)) {
        if (return_quotient) {
            MPI res = bi_init(1);
            bi_set(res, 0);
            return bi_result_make(res, BI_OK);
        } else {
            MPI res = bi_init_and_copy(u);
            return bi_result_make(res, BI_OK);
        }
    }

    if (v->words == 1) {
        return __bi_div_imm(u, v->data[0], return_quotient);
    }

    // D0: Define
    MPI Ustruct = bi_pad_words(u, 1);
    uint32_t m = u->words;
    MPI Vstruct = bi_init_and_copy(v);
    uint32_t n = Vstruct->words;
    const uint64_t B = 1ull << 32;
    MPI Qstruct = bi_init(m - n + 1);

    if (m < n || n <= 1 || v->data[n - 1] == 0) {
        printf("Bad operands in knuth_d. m=%d, n=%d", m, n);
        return bi_result_error(BI_BAD_OPERANDS);
    }

    // D1: Normalise
    uint32_t D = leading_zeros(Vstruct->data[n - 1]);
    MPI Utmp = bi_shift_left(Ustruct, D);
    MPI Vtmp = bi_shift_left(Vstruct, D);
    bi_copy(Utmp, Ustruct);
    bi_copy(Vtmp, Vstruct);
    bi_free(Utmp);
    bi_free(Vtmp);
    Utmp = bi_pad_words(Ustruct, 1);
    bi_copy(Utmp, Ustruct);
    bi_free(Utmp);

    // D2/D7: Loop setup
    uint32_t *U = Ustruct->data;
    uint32_t *V = Vstruct->data;
    uint32_t *Q = Qstruct->data;

    for (int32_t j = m - n; j >= 0; j--) {

        // D3: Calculate q_hat
        uint64_t div_num = (uint64_t)U[n + j] * B + U[n - 1 + j];
        uint64_t div_den = V[n - 1];
        uint64_t q_hat = div_num / div_den;
        uint64_t r_hat = div_num % div_den;

    again:
        if (q_hat == B ||
            q_hat * (uint64_t)V[n - 2] > (r_hat * B + (uint64_t)U[n - 2 + j])) {
            q_hat--;
            r_hat += V[n - 1];

            if (r_hat < B) {
                goto again;
            }
        }

        // D4: Multiply and subtract
        int64_t k = 0;
        int64_t t;
        for (uint32_t i = 0; i < n; i++) {
            uint64_t p = q_hat * (uint64_t)V[i];
            t = U[i + j] - k - (p & 0xFFFFFFFFLL);
            U[i + j] = t;
            k = (p >> 32) - (t >> 32);
        }
        t = U[j + n] - k;
        U[j + n] = t;

        // D5: Test remainder
        Q[j] = (uint32_t)q_hat;

        if (t < 0) {
            // D6: Add back
            Q[j]--;

            k = 0;
            for (uint32_t i = 0; i < n; i++) {
                t = (uint64_t)U[i + j] + V[i] + k;
                U[i + j] = t;
                k = t >> 32;
            }

            // deliberately ignore overflow here, it's
            // balanced out by the borrow in the subtraction
            // above
            U[j + n] += k;
        }
    }

    bi_free(Vstruct);

    if (return_quotient) {
        bi_free(Ustruct);

        bi_squeeze(Qstruct);

        return bi_result_make(Qstruct, BI_OK);
    } else {
        // D8: Unnormalise
        bi_free(Qstruct);
        MPI res = bi_shift_right(Ustruct, D);
        bi_free(Ustruct);

        bi_squeeze(res);
        return bi_result_make(res, BI_OK);
    }
}

MPI knuth_d(MPI u, MPI v, bool return_quotient) {

    __bi_result_t res = __knuth_d(u, v, return_quotient);

    if (res.code != BI_OK) {
        printf("ERROR in knuth_d, code: %d", res.code);
        exit(1);
    }

    return res.x;
}

__bi_result_t __bi_eucl_div_old(MPI x, MPI y) {
    if (bi_eq_val(y, 0)) {
        return bi_result_error(BI_DIV_ZERO);
    }

    if (bi_gt(y, x)) {
        MPI res = bi_init(1);
        bi_set(res, 0u);
        return bi_result_make(res, BI_OK);
    }

    uint32_t y_orig_words;
    bool y_was_squeezed = false;
    if (x->words > y->words) {
        y_was_squeezed = true;
        y_orig_words = y->words;
        MPI tmp = bi_pad_words(y, x->words - y->words);
        bi_copy(tmp, y);
        bi_free(tmp);
    }

    MPI q = bi_init_like(x);
    MPI r = bi_init_like(x);

    uint32_t div_bits = 32u * x->words;
    for (int32_t i = div_bits - 1; i >= 0; i--) {
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
            MPI tmp = bi_pad_words(y, y_orig_words - y->words);
            bi_copy(tmp, y);
            bi_free(tmp);
        }
    }

    bi_free(r);
    bi_squeeze(q);
    return bi_result_make(q, BI_OK);
    ;
}

MPI bi_eucl_div(MPI a, MPI b) {
    __bi_result_t res = __knuth_d(a, b, true);

    if (res.code != BI_OK) {
        printf("bi_eucl_div failed with error code: %d", res.code);
        if (res.x != NULL) {
            bi_free(res.x);
        }
        exit(1);
    }

    return res.x;
}

void bi_print(MPI x) {
    printf("0x");
    for (int32_t i = x->words - 1; i > 0; i--) {
        printf("%08x", x->data[i]);
    }
    printf("%08x", x->data[0]);
}

void bi_printf(MPI x, FILE *fp) {
    for (int32_t i = x->words - 1; i >= 0; i--) {
        fprintf(fp, "%08x", x->data[i]);
    }
}

MPI from_hex_str(char *str) {
    uint32_t len = strlen(str);
    uint32_t words = len / 8;

    MPI res = bi_init(words);

    // note - in a string representation,
    // left->right is most significant -> least
    // significant. But, our lib is idx 0 = least
    // significant, increasing. So the first word
    // in the string will be the most significant
    // word in the output, which goes at the end.
    char *endptr;
    for (uint32_t i = 0; i < words; i++) {
        // extract the substr for the curr word
        char slice[9];
        strncpy(slice, str + i, ((uint64_t)str + (i + 8ull)));
        slice[8] = '\0';

        // parse the substr
        res->data[res->words - i - 1] = strtoul(slice, &endptr, 16);

        if (*endptr != '\0') {
            printf("Failed parsing MPI string %s\nExiting.", str);
            exit(1);
        }
    }

    return res;
}

void bi_inc(MPI x) {
    uint32_t i = 0;
    while (x->data[i] == 0xFFFFFFFF && i < x->words) {
        x->data[i] = 0u;
        i++;
    }

    if (i == x->words) {
        MPI tmp = bi_pad_words(x, 1);
        bi_copy(tmp, x);
        bi_free(tmp);

        x->data[x->words - 1] = 1u;
    } else {
        x->data[i]++;
    }

    bi_squeeze(x);
}

void bi_dec(MPI x) {
    uint32_t i = 0;
    while (x->data[i] == 0u && i < x->words) {
        x->data[i] = 0xFFFFFFFF;
        i++;
    }

    if (x->data[i] != 0) {
        x->data[i]--;
    }

    bi_squeeze(x);
}
MPI bi_and(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words));

    for (uint32_t i = 0; i < min(a->words, b->words); i++) {
        res->data[i] = a->data[i] & b->data[i];
    }

    bi_squeeze(res);

    return res;
}
MPI bi_or(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words));

    for (uint32_t i = 0; i < min(a->words, b->words); i++) {
        res->data[i] = a->data[i] | b->data[i];
    }

    if (a->words > b->words) {
        for (uint32_t i = b->words; i < a->words; i++) {
            res->data[i] = a->data[i];
        }
    } else if (a->words < b->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
            res->data[i] = b->data[i];
        }
    }

    bi_squeeze(res);

    return res;
}
MPI bi_xor(MPI a, MPI b) {
    MPI res = bi_init(max(a->words, b->words));

    for (uint32_t i = 0; i < min(a->words, b->words); i++) {
        res->data[i] = a->data[i] ^ b->data[i];
    }

    if (a->words > b->words) {
        for (uint32_t i = b->words; i < a->words; i++) {
            res->data[i] = a->data[i];
        }
    } else if (a->words < b->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
            res->data[i] = b->data[i];
        }
    }

    bi_squeeze(res);

    return res;
}
MPI bi_not(MPI a) {
    MPI res = bi_init_like(a);

    for (uint32_t i = 0; i < a->words; i++) {
        res->data[i] = ~a->data[i];
    }

    bi_squeeze(res);
    return res;
}

MPI bi_shift_left(MPI a, uint32_t n) {
    if (n == 0) {
        return bi_init_and_copy(a);
    }

    bi_squeeze(a);
    if (bi_eq_val(a, 0)) {
        return bi_init(1);
    }

    uint32_t offset_words = n / 32;
    uint32_t offset_mod = n % 32;

    MPI res = bi_init(a->words + offset_words + 1);

    for (uint32_t i = offset_words; i < res->words; i++) {
        if (i < a->words + offset_words) {
            uint32_t upper_fetch_word = a->data[i - offset_words];
            res->data[i] += upper_fetch_word << offset_mod;
        }

        if (offset_mod && i > offset_words) {
            uint32_t lower_fetch_word = a->data[i - offset_words - 1];
            res->data[i] += lower_fetch_word >> (32 - offset_mod);
        }
    }

    bi_squeeze(res);
    return res;
}

MPI bi_shift_right(MPI a, uint32_t n) {
    if (n == 0) {
        return bi_init_and_copy(a);
    }

    if (n >= 32 * a->words) {
        return bi_init(1);
    }

    bi_squeeze(a);
    if (bi_eq_val(a, 0)) {
        return bi_init(1);
    }

    MPI res = bi_init_like(a);

    uint32_t offset_words = n / 32;
    uint32_t offset_mod = n % 32;

    for (uint32_t i = 0; i < res->words - offset_words; i++) {
        uint32_t lower_fetch_word = a->data[i + offset_words + 0];
        res->data[i] = ((uint64_t)lower_fetch_word >> offset_mod);

        if (i < a->words - offset_words - 1 && offset_mod) {
            uint32_t upper_fetch_word = a->data[i + offset_words + 1];
            res->data[i] += (upper_fetch_word << (32 - offset_mod));
        }
    }

    bi_squeeze(res);
    return res;
}

// a^b % n
MPI bi_mod_exp(MPI a, MPI b, MPI n) {
    if (bi_eq_val(n, 1u)) {
        MPI res = bi_init(1u);
        return res;
    }

    if (bi_eq_val(b, 0u)) {
        MPI res = bi_init(1u);
        bi_set(res, 1u);
        return res;
    }

    MPI res = bi_init(1u);
    bi_set(res, 1u);
    MPI base_tmp = bi_mod(a, n);
    MPI b_copy = bi_init_and_copy(b);

    while (!bi_eq_val(b_copy, 0u)) {
        if (!bi_even(b_copy)) {
            // res = (res * a) % n
            // TODO: replace with modular multiplication
            MPI res_base = bi_mul(res, base_tmp);
            MPI res_tmp = bi_mod(res_base, n);
            bi_copy(res_tmp, res);

            bi_dec(b_copy);

            bi_free(res_base);
            bi_free(res_tmp);
        } else {
            // a = a^2 % n
            MPI a_squared = bi_mul(base_tmp, base_tmp);
            MPI base_tmp_tmp = bi_mod(a_squared, n);
            bi_copy(base_tmp_tmp, base_tmp);
            bi_free(a_squared);
            bi_free(base_tmp_tmp);

            // b >>= 1
            MPI b_tmp = bi_shift_right(b_copy, 1u);
            bi_copy(b_tmp, b_copy);
            bi_free(b_tmp);
        }
    }

    bi_free(base_tmp);
    bi_free(b_copy);

    return res;
}

bool bi_lt(MPI a, MPI b) {
    if (a->words > b->words) {
        for (uint32_t i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return false;
            }
        }
    } else if (b->words > a->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
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

    for (int32_t i = a->words - 1; i >= 0; i--) {
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
        for (uint32_t i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return false;
            }
        }
    } else if (b->words > a->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
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

    for (int32_t i = a->words - 1; i >= 0; i--) {
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
    for (uint32_t i = 0; i < min(a->words, b->words); i++) {
        if (a->data[i] != b->data[i])
            return false;
    }

    if (a->words > b->words) {
        for (uint32_t i = b->words; i < a->words; i++) {
            if (a->data[i] != 0u) {
                return false;
            }
        }
    } else if (b->words > a->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
            if (b->data[i] != 0u) {
                return false;
            }
        }
    }

    return true;
}

bool bi_eq_val(MPI a, uint32_t b) {
    for (uint32_t i = 1; i < a->words; i++) {
        if (a->data[i] != 0u)
            return false;
    }

    return a->data[0] == b;
}

bool bi_gt(MPI a, MPI b) {
    if (a->words > b->words) {
        for (uint32_t i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return true;
            }
        }
    } else if (b->words > a->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
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

    for (uint32_t i = 0; i < a->words; i++) {
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
        for (uint32_t i = b->words; i < a->words; i++) {
            if (a->data[i] > 0u) {
                return true;
            }
        }
    } else if (b->words > a->words) {
        for (uint32_t i = a->words; i < b->words; i++) {
            if (b->data[i] > 0u) {
                return false;
            }
        }
    }

    for (uint32_t i = 0; i < a->words; i++) {
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

MPI bi_pad_words(MPI x, uint32_t n) {
    MPI res = bi_init(x->words + n);
    res->words = x->words + n;

    for (uint32_t i = 0; i < x->words; i++) {
        res->data[i] = x->data[i];
    }

    return res;
}

MPI bi_pad_words_from_bottom(MPI x, uint32_t n) {

    MPI res = bi_init(x->words + n);

    for (uint32_t i = 0; i < x->words; i++) {
        res->data[i + n] = x->data[i];
    }

    return res;
}

void bi_squeeze(MPI x) {
    uint32_t new_words = x->words;

    while (new_words > 1 && x->data[new_words - 1] == 0u) {
        new_words--;
    }

    if (new_words == x->words) {
        return;
    }

    x->data = realloc(x->data, new_words * sizeof(uint32_t));
    x->words = new_words;
}

// slices a from start to end, exclusive at both ends
__bi_result_t __bi_slice(MPI a, uint32_t start, uint32_t end) {
    if (end <= start || start >= a->words * 32u) {
        return bi_result_error(BI_BAD_OPERANDS);
    }

    MPI res = bi_init(end - start + 1);

    for (uint32_t i = start; i <= end; i++) {
        res->data[i] = a->data[i + start];
    }

    return bi_result_make(res, BI_OK);
}

MPI bi_slice(MPI a, uint32_t start, uint32_t end) {

    __bi_result_t res = __bi_slice(a, start, end);

    if (res.code != BI_OK) {
        printf("Error in bi_slice. Error code: %d\n", res.code);
        exit(1);
    }

    return res.x;
}

__bi_result_code_t __bi_copy_word_range(MPI src, MPI target,
                                        uint32_t src_start_idx,
                                        uint32_t target_start_idx,
                                        uint32_t copy_words) {
    if (src_start_idx + copy_words >= src->words ||
        target_start_idx + copy_words >= target->words) {
        return BI_BAD_OPERANDS;
    }

    for (uint32_t i = 0; i < copy_words; i++) {

        target->data[target_start_idx + i] = src->data[src_start_idx + i];
    }

    return BI_OK;
}

void bi_copy_word_range(MPI src, MPI target, uint32_t src_start_idx,
                        uint32_t target_start_idx, uint32_t copy_words) {
    __bi_result_code_t res_code = __bi_copy_word_range(
        src, target, src_start_idx, target_start_idx, copy_words);

    if (res_code != BI_OK) {
        printf("Error in bi_copy_word_range. Error code: %d\n", res_code);
        exit(1);
    }
}

__bi_result_code_t __bi_add_to_range(MPI src, MPI target,
                                     uint32_t src_start_idx,
                                     uint32_t target_start_idx,
                                     uint32_t range_words) {
    if (src_start_idx + range_words >= src->words ||
        target_start_idx + range_words >= target->words) {
        return BI_BAD_OPERANDS;
    }

    uint64_t carry = 0;
    for (uint32_t i = 0; i < range_words; i++) {
        target->data[target_start_idx + i] += src->data[src_start_idx + i];
        uint64_t res = (uint64_t)target->data[target_start_idx + i] +
                       (uint64_t)src->data[target_start_idx + i] + carry;
        target->data[target_start_idx + i] = (uint32_t)res;
        carry = res >> 32u;
    }

    // TODO: what happens if carry != 0 here??

    return BI_OK;
}

void bi_add_to_range(MPI src, MPI target, uint32_t src_start_idx,
                     uint32_t target_start_idx, uint32_t range_words) {
    __bi_result_code_t res_code = __bi_add_to_range(
        src, target, src_start_idx, target_start_idx, range_words);

    if (res_code != BI_OK) {
        printf("Error in bi_copy_word_range. Error code: %d\n", res_code);
        exit(1);
    }
}

__bi_result_t __bi_div_imm(MPI a, uint32_t b, bool return_quotient) {
    if (b == 0) {
        return bi_result_error(BI_DIV_ZERO);
    }

    MPI q = bi_init_like(a);
    uint64_t r = 0;

    for (int i = a->words - 1; i >= 0; i--) {
        uint64_t x = (r << 32) | a->data[i];
        q->data[i] = x / b;
        r = x % b;
    }

    if (return_quotient) {
        return bi_result_make(q, BI_OK);
    } else {
        bi_free(q);
        MPI rem = bi_init(1);
        rem->data[0] = (uint32_t)r;
        return bi_result_make(rem, BI_OK);
    }
}

MPI bi_mod_imm(MPI a, uint32_t b) {

    __bi_result_t res = __bi_div_imm(a, b, false);

    if (res.code != BI_OK) {
        printf("error in bi_mod_imm. code=%d", res.code);
        exit(1);
    }

    return res.x;
}

MPI bi_eucl_div_imm(MPI a, uint32_t b) {

    __bi_result_t res = __bi_div_imm(a, b, true);

    if (res.code != BI_OK) {
        printf("error in bi_mod_imm. code=%d", res.code);
        exit(1);
    }

    return res.x;
}

uint64_t trailing_zeros(MPI a) {
    uint64_t sum = 0;

    for (uint32_t i = 0; i < a->words; i++) {
        uint32_t v = a->data[i];

        if (v == 0) {
            sum += 32;

        } else {
            uint64_t curr_word_ctz = __builtin_ctz(v);
            sum += curr_word_ctz;
            break;
        }
    }

    return sum;
}

MPI bi_gcd(MPI a, MPI b) {
    // gcd(x, 0) = x
    if (bi_eq_val(a, 0) && !bi_eq_val(b, 0)) {
        return bi_init_and_copy(b);
    }
    if (!bi_eq_val(a, 0) && bi_eq_val(b, 0)) {
        return bi_init_and_copy(a);
    }

    // define gcd(0, 0) = 0
    if (bi_eq_val(a, 0) && bi_eq_val(b, 0)) {
        return bi_init(1);
    }

    bi_squeeze(a);
    bi_squeeze(b);

    MPI tmp = bi_or(a, b);
    uint32_t shift = trailing_zeros(tmp);
    bi_free(tmp);

    MPI a_tmp = bi_shift_right(a, trailing_zeros(a));
    MPI b_tmp = bi_init_and_copy(b);

    do {
        tmp = bi_shift_right(b_tmp, trailing_zeros(b_tmp));
        bi_free(b_tmp);
        b_tmp = bi_init_and_copy(tmp);
        bi_free(tmp);

        if (bi_gt(a_tmp, b_tmp)) {
            tmp = bi_init_and_copy(a_tmp);
            bi_free(a_tmp);
            a_tmp = bi_init_and_copy(b_tmp);
            bi_free(b_tmp);
            b_tmp = bi_init_and_copy(tmp);
            bi_free(tmp);
        }

        tmp = bi_sub(b_tmp, a_tmp);
        bi_free(b_tmp);
        b_tmp = bi_init_and_copy(tmp);
        bi_free(tmp);

    } while (!bi_eq_val(b_tmp, 0));

    MPI res = bi_shift_left(a_tmp, shift);

    bi_free(a_tmp);
    bi_free(b_tmp);

    return res;
}

MPI bi_lcm(MPI a, MPI b) {
    // lcm(x, 0) = x
    // this includes lcm(0, 0), which is undefined mathematically
    // but generally defined as 0 in software libs
    if (bi_eq_val(a, 0) || bi_eq_val(b, 0)) {
        return bi_init(1);
    }

    MPI gcd_ab = bi_gcd(a, b);
    MPI res = bi_eucl_div(a, gcd_ab);
    MPI res2 = bi_mul(res, b);

    bi_free(gcd_ab);
    bi_free(res);

    return res2;
}
