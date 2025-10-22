#include <bigint/bigint.h>

sMPI make_small_signed(uint32_t val, bool positive) {
    sMPI res;
    res.val = bi_init(1);
    bi_set(res.val, val);
    res.positive = positive;

    return res;
}

sMPI from_unsigned(MPI x) {
    sMPI res;
    res.val = bi_init_and_copy(x);
    res.positive = true;

    return res;
}

sMPI signed_add(sMPI a, sMPI b) {
    sMPI res;
    if (a.positive == b.positive) {
        res.val = bi_add(a.val, b.val);
        res.positive = a.positive;
    } else {
        if (bi_gt(a.val, b.val)) {
            res.val = bi_sub(a.val, b.val);
            res.positive = a.positive;
        } else {
            res.val = bi_sub(b.val, a.val);
            res.positive = !a.positive;
        }
    }

    return res;
}

bool signed_eq_val(sMPI x, uint32_t val, bool positive) {
    return x.positive == positive && bi_eq_val(x.val, val);
}

void signed_free(sMPI x) { bi_free(x.val); }

MPI to_unsigned(sMPI x) {
    MPI res = bi_init_and_copy(x.val);

    return res;
}

sMPI signed_sub(sMPI a, sMPI b) {
    sMPI res;

    if (a.positive == b.positive) {
        if (bi_gt(a.val, b.val)) {
            res.val = bi_sub(a.val, b.val);
            res.positive = a.positive;
        } else {
            res.val = bi_sub(b.val, a.val);
            res.positive = !a.positive;
        }
    } else {
        res.val = bi_add(a.val, b.val);
        res.positive = a.positive;
    }

    return res;
}

sMPI signed_mul(sMPI a, sMPI b) {
    sMPI res;
    res.val = bi_mul(a.val, b.val);
    res.positive = a.positive == b.positive;

    return res;
}

sMPI signed_init_copy(sMPI src) {
    sMPI res;
    res.val = bi_init_and_copy(src.val);
    res.positive = src.positive;

    return res;
}

void signed_free_init_copy(sMPI src, sMPI *target) {
    bi_free(target->val);
    *target = signed_init_copy(src);
}

void signed_eucl_div(sMPI a, sMPI b, sMPI *q, MPI *r) {
    MPI r_;
    bi_eucl_div(a.val, b.val, &q->val, &r_);
    q->positive = a.positive == b.positive || bi_eq_val(q->val, 0);

    if (!q->positive && !bi_eq_val(r_, 0)) {
        signed_dec(q);

        MPI r__ = bi_sub(b.val, r_);
        bi_free(r_);
        r_ = r__;
    }

    if (r != NULL) {
        *r = r_;
    }
}

ext_euc_res_t ext_euc(MPI a, MPI b) {
    sMPI r = {NULL, true}, s = {NULL, true}, t = {NULL, true},
         old_r = {NULL, true}, old_s = {NULL, true}, old_t = {NULL, true},
         quotient = {NULL, true}, tmp1 = {NULL, true}, tmp2 = {NULL, true};

    old_r.val = bi_init_and_copy(a);
    r.val = bi_init_and_copy(b);
    old_s.val = bi_init_like(a);
    bi_set(old_s.val, 1);
    s.val = bi_init_like(a);
    old_t.val = bi_init_like(a);
    t.val = bi_init_like(a);
    bi_set(t.val, 1);

    while (!bi_eq_val(r.val, 0)) {
        signed_eucl_div(old_r, r, &quotient, NULL);

        // (old_r, r) := (r, old_r − quotient × r)
        tmp1 = signed_init_copy(r);
        tmp2 = signed_mul(quotient, r);
        signed_free(r);
        r = signed_sub(old_r, tmp2);
        signed_free_init_copy(tmp1, &old_r);
        bi_free(tmp1.val);
        bi_free(tmp2.val);

        // (old_s, s) := (s, old_s − quotient × s)
        tmp1 = signed_init_copy(s);
        tmp2 = signed_mul(quotient, s);
        bi_free(s.val);
        s = signed_sub(old_s, tmp2);
        signed_free_init_copy(tmp1, &old_s);
        bi_free(tmp1.val);
        bi_free(tmp2.val);

        // (old_t, t) := (t, old_t − quotient × t)
        tmp1 = signed_init_copy(t);
        tmp2 = signed_mul(quotient, t);
        bi_free(t.val);
        t = signed_sub(old_t, tmp2);
        signed_free_init_copy(tmp1, &old_t);
        bi_free(tmp1.val);
        bi_free(tmp2.val);
        bi_free(quotient.val);
    }

    ext_euc_res_t res;
    res.bez_x = old_s;
    res.bez_y = old_t;
    res.gcd = old_r;

    bi_free(r.val);
    bi_free(s.val);
    bi_free(t.val);
    return res;
}

void signed_inc(sMPI* a) {
    if (signed_eq_val(*a, 1, false)) {
        bi_set(a->val, 0);
        a->positive = true;
    } else if (a->positive) {
        bi_inc(a->val);
    } else {
        bi_dec(a->val);
    }
}
void signed_dec(sMPI* a) {
    if (signed_eq_val(*a, 0, true)) {
        bi_set(a->val, 1);
        a->positive = false;
    } else if (a->positive) {
        bi_dec(a->val);
    } else {
        bi_inc(a->val);
    }
}