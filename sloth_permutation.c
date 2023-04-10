#include "sloth_permutation.h"
#include <stdlib.h>

SlothPermutation* sloth_permutation_new(void) {
    SlothPermutation* sp = malloc(sizeof(SlothPermutation));
    if (!sp) {
        return NULL;
    }
    mpz_init_set_str(sp->p, "170082004324204494273811327264862981553264701145937538369570764779791492622392118654022654452947093285873855529044371650895045691292912712699015605832276411308653107069798639938826015099738961427172366594187783204437869906954750443653318078358839409699824714551430573905637228307966826784684174483831608534979", 10);
    return sp;
}

void sloth_permutation_set_prime(SlothPermutation* sp, const char* prime_str) {
    mpz_init_set_str(sp->p, prime_str, 10);
}

void sloth_permutation_free(SlothPermutation* sp) {
    if (sp) {
        mpz_clear(sp->p);
        free(sp);
    }
}

bool sloth_permutation_sqrt_mod_p_verify(SlothPermutation* sp, mpz_t y, mpz_t x, mpz_t p) {
    mpz_t temp;
    mpz_init(temp);
    mpz_powm_ui(temp, y, 2, p);
    bool result = mpz_cmp(temp, x) == 0;
    mpz_clear(temp);
    return result;
}

void sloth_permutation_mod(SlothPermutation* sp, mpz_t result, mpz_t x, mpz_t y) {
    mpz_tdiv_r(result, x, y);
}

void sloth_permutation_fast_pow(SlothPermutation* sp, mpz_t result, mpz_t base, mpz_t exponent, mpz_t modulus) {
    mpz_powm(result, base, exponent, modulus);
}

bool sloth_permutation_quad_res(SlothPermutation* sp, mpz_t x) {
    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, sp->p, 1);
    mpz_tdiv_q_ui(temp, temp, 2);
    sloth_permutation_fast_pow
(sp, temp, x, temp, sp->p);
    bool result = mpz_cmp_ui(temp, 1) == 0;
    mpz_clear(temp);
    return result;
}

void sloth_permutation_mod_sqrt_op(SlothPermutation* sp, mpz_t result, mpz_t x) {
    mpz_t temp;
    mpz_init(temp);
    if (sloth_permutation_quad_res(sp, x)) {
        mpz_add_ui(temp, sp->p, 1);
        mpz_tdiv_q_ui(temp, temp, 4);
        sloth_permutation_fast_pow(sp, result, x, temp, sp->p);
    } else {
        mpz_sub(temp, sp->p, x);
        mpz_mod(temp, temp, sp->p);
        mpz_add_ui(temp, sp->p, 1);
        mpz_tdiv_q_ui(temp, temp, 4);
        sloth_permutation_fast_pow(sp, result, temp, temp, sp->p);
    }
    mpz_clear(temp);
}

void sloth_generate_proof_vdf(SlothPermutation* sp, mpz_t result, mpz_t x, size_t t) {
    mpz_mod(x, x, sp->p);
    for (size_t i = 0; i < t; i++) {
        sloth_permutation_mod_sqrt_op(sp, x, x);
    }
    mpz_set(result, x);
}

bool sloth_verify_proof_vdf(SlothPermutation* sp, mpz_t y, mpz_t x, size_t t) {
    mpz_mod(x, x, sp->p);
    for (size_t i = 0; i < t; i++) {
        mpz_powm_ui(y, y, 2, sp->p);
    }
    if (!sloth_permutation_quad_res(sp, y)) {
        mpz_sub(y, sp->p, y);
        mpz_mod(y, y, sp->p);
    }
    bool result = mpz_cmp(x, y) == 0 || mpz_cmp(x, y) == mpz_cmp(sp->p, y);
    return result;
}

uint64_t readBigUInt64LE(const uint8_t* buffer, size_t offset) {
    uint64_t lo = buffer[offset] +
                  buffer[++offset] * (uint64_t) 0x100 +
                  buffer[++offset] * (uint64_t) 0x10000 +
                  buffer[++offset] * (uint64_t) 0x1000000;

    uint64_t hi = buffer[++offset] +
                  buffer[++offset] * (uint64_t) 0x100 +
                  buffer[++offset] * (uint64_t) 0x10000 +
                  buffer[++offset] * (uint64_t) 0x1000000;

    return lo + (hi << 32);
}

void writeBigUInt64LE(uint64_t x, uint8_t* buffer, size_t offset) {
    for (size_t i = 0; i < 8; i++) {
        buffer[offset + i] = (uint8_t) (x & 0xFF);
        x >>= 8;
    }
}

void readBigUIntLE(mpz_t result, const uint8_t* buffer, size_t byteLen, size_t offset) {
    mpz_set_ui(result, 0);

    for (size_t i = 0; i < byteLen; i++) {
        mpz_t tmp;
        mpz_init(tmp);
        mpz_ui_pow_ui(tmp, 2, (unsigned long) (i * 8));
        mpz_mul_ui(tmp, tmp, buffer[offset + i]);
        mpz_add(result, result, tmp);
        mpz_clear(tmp);
    }
}

void writeBigUIntLE(mpz_t x, uint8_t* buffer, size_t byteLen, size_t offset) {
    mpz_t tmp;
    mpz_init(tmp);
    mpz_set(tmp, x);

    for (size_t i = 0; i < byteLen; i++) {
        buffer[offset + i] = (uint8_t) mpz_mod_ui(NULL, tmp, 256);
        mpz_fdiv_q_ui(tmp, tmp, 256);
    }

    mpz_clear(tmp);
}
