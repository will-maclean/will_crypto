#include <bigint/bigint.h>

sMPI signed_init(uint32_t words) {
    sMPI res;
    res.val = bi_init(words);
    res.positive = true;

    return res;
}

sMPI make_small_signed(uint32_t val, bool positive) {
    sMPI res;
    res.val = bi_init(1);
    bi_set(res.val, val);
    res.positive = positive;

    return res;
}

sMPI from_unsigned(MPI x, bool positive) {
    sMPI res;
    res.val = bi_init_and_copy(x);
    res.positive = positive;

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

    if (bi_eq_val(res.val, 0)) {
        res.positive = true;
    }

    return res;
}
bool signed_eq(sMPI a, sMPI b) {
    return a.positive == b.positive && bi_eq(a.val, b.val);
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

    if (bi_eq_val(res.val, 0)) {
        res.positive = true;
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

    if (!a.positive && !bi_eq_val(r_, 0)) {
        bi_inc(q->val);

        MPI r__ = bi_sub(b.val, r_);
        bi_free(r_);
        r_ = r__;
    }

    if (r != NULL) {
        *r = r_;
    }
}

sMPI safe_signed_half(sMPI a) {
    sMPI res;
    if(signed_eq_val(a, 0, true)){
        return make_small_signed(0, true);
    } else if (a.positive) {
        res.val = bi_shift_right(a.val, 1);
        res.positive = true;

        return res;
    } else {
        sMPI a_cpy = signed_init_copy(a);

        bi_inc(a_cpy.val);

        res.val = bi_shift_right(a_cpy.val, 1);
        res.positive = false;

        signed_free(a_cpy);

        return res;
    }
}

ext_euc_res_t ext_euc(MPI a, MPI b) {
    if (bi_eq(a, b)) {
        ext_euc_res_t res;

        res.gcd = from_unsigned(a, true);
        res.bez_x = make_small_signed(1, true);
        res.bez_y = make_small_signed(0, true);

        return res;
    }

    sMPI x = from_unsigned(a, true);
    sMPI y = from_unsigned(b, true);

    sMPI g = make_small_signed(1, true);

    MPI tmp;
    sMPI stmp;
    uint32_t xy_iter = 0;
    while (signed_even(x) && signed_even(y)) {
        // x /= 2
        stmp = safe_signed_half(x);
        signed_free_init_copy(stmp, &x);

        // y /= 2
        stmp = safe_signed_half(y);
        signed_free_init_copy(stmp, &y);

        // g *= 2
        tmp = bi_shift_left(g.val, 1);
        bi_free(g.val);
        g.val = bi_init_and_copy(tmp);
        bi_free(tmp);

        // printf("xy iter %d\n\tx=", xy_iter++);
        // signed_print(x);
        // printf("\n\ty=");
        // signed_print(y);
        // printf("\n\tg=");
        // signed_print(g);
        // printf("\n");
    }

    sMPI u = signed_init_copy(x);
    sMPI v = signed_init_copy(y);
    sMPI A = make_small_signed(1, true);
    sMPI B = make_small_signed(0, true);
    sMPI C = make_small_signed(0, true);
    sMPI D = make_small_signed(1, true);

    uint32_t outer_loop = 0;
    while (!signed_eq_val(u, 0, true)) {
        while (signed_even(u)) {
            // u /= 2
            stmp = safe_signed_half(u);
            signed_free_init_copy(stmp, &u);
            signed_free(stmp);

            if (signed_even(A) && signed_even(B)) {
                // A /= 2
                stmp = safe_signed_half(A);
                signed_free_init_copy(stmp, &A);
                signed_free(stmp);

                // B /= 2
                stmp = safe_signed_half(B);
                signed_free_init_copy(stmp, &B);
                signed_free(stmp);

            } else {
                // A = (A + y) / 2
                sMPI aplusy = signed_add(A, y);
                stmp = safe_signed_half(aplusy);
                signed_free_init_copy(stmp, &A);
                signed_free(aplusy);
                signed_free(stmp);

                // B = (B - x) / 2
                sMPI bminusx = signed_sub(B, x);
                stmp = safe_signed_half(bminusx);
                signed_free_init_copy(stmp, &B);
                signed_free(bminusx);
                signed_free(stmp);
            }
        }

        while (signed_even(v)) {
            // v /= 2
            stmp = safe_signed_half(v);
            signed_free_init_copy(stmp, &v);
            signed_free(stmp);

            if (signed_even(C) && signed_even(D)) {
                // C /= 2
                stmp = safe_signed_half(C);
                signed_free_init_copy(stmp, &C);
                signed_free(stmp);

                // D /= 2
                stmp = safe_signed_half(D);
                signed_free_init_copy(stmp, &D);
                signed_free(stmp);
            } else {
                // C = (C + y) / 2
                sMPI cplusy = signed_add(C, y);
                stmp = safe_signed_half(cplusy);
                signed_free_init_copy(stmp, &C);
                signed_free(stmp);

                // D = (D - x) / 2
                sMPI dminusx = signed_sub(D, x);
                stmp = safe_signed_half(dminusx);
                signed_free_init_copy(stmp, &D);
                signed_free(stmp);
            }
        }

        if (signed_ge(u, v)) {
            // u -= v
            stmp = signed_sub(u, v);
            signed_free_init_copy(stmp, &u);
            signed_free(stmp);

            // A -= C
            stmp = signed_sub(A, C);
            signed_free_init_copy(stmp, &A);
            signed_free(stmp);

            // B -= D
            stmp = signed_sub(B, D);
            signed_free_init_copy(stmp, &B);
            signed_free(stmp);
        } else {

            // v -= u
            sMPI stmp = signed_sub(v, u);
            signed_free_init_copy(stmp, &v);
            signed_free(stmp);

            //  C -= A
            stmp = signed_sub(C, A);
            signed_free_init_copy(stmp, &C);
            signed_free(stmp);

            //  D -= B
            stmp = signed_sub(D, B);
            signed_free_init_copy(stmp, &D);
            signed_free(stmp);
        }

        // printf("u outer iter %d\n\tu=", outer_loop++);
        // signed_print(u);
        // printf("\n\tv=");
        // signed_print(v);
        // printf("\n\tA=");
        // signed_print(A);
        // printf("\n\tB=");
        // signed_print(B);
        // printf("\n\tC=");
        // signed_print(C);
        // printf("\n\tD=");
        // signed_print(D);
        // printf("\n");
    }

    ext_euc_res_t res;
    res.bez_x = signed_init_copy(C);
    res.bez_y = signed_init_copy(D);
    res.gcd = signed_mul(g, v);

    // printf("gcd=");
    // signed_print(res.gcd);
    // printf("\nbez_x=");
    // signed_print(res.bez_x);
    // printf("\nbez_y=");
    // signed_print(res.bez_y);

    signed_free(x);
    signed_free(y);
    signed_free(u);
    signed_free(v);
    signed_free(A);
    signed_free(B);
    signed_free(C);
    signed_free(D);

    return res;
}

void signed_inc(sMPI *a) {
    if (signed_eq_val(*a, 1, false)) {
        bi_set(a->val, 0);
        a->positive = true;
    } else if (a->positive) {
        bi_inc(a->val);
    } else {
        bi_dec(a->val);
    }
}
void signed_dec(sMPI *a) {
    if (signed_eq_val(*a, 0, true)) {
        bi_set(a->val, 1);
        a->positive = false;
    } else if (a->positive) {
        bi_dec(a->val);
    } else {
        bi_inc(a->val);
    }
}

void signed_print(sMPI a) {
    printf(a.positive ? "+" : "-");
    bi_print(a.val);
}

void signed_printf(sMPI a, FILE *fp) {
    fprintf(fp, a.positive ? "+" : "-");
    bi_printf(a.val, fp);
}

bool signed_even(sMPI a) { return bi_even(a.val); }

bool signed_ge(sMPI a, sMPI b) {
    if (a.positive && !b.positive) {
        return true;
    } else if (!a.positive && b.positive) {
        return false;
    } else if (a.positive && b.positive) {
        return bi_ge(a.val, b.val);
    } else {
        return bi_ge(b.val, a.val);
    }
}
