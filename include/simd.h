/*
Header comment:
    File:       simd.h
    Project:    lang
    Created:    2026-08-21

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

*/


#ifndef SIMD_H
#define SIMD_H

#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1 (optional)
#include <immintrin.h> // BMI2

// Constants:
extern __m128i SIMD_ALL_BITS;
extern __m128i SIMD_SIGN_BIT;
extern __m128i SIMD_ASCII_MAX;
extern __m128i SIMD_NON_WHITESPACE;
extern __m128i SIMD_ZERO;
extern __m128i SIMD_NINE;
extern __m128i SIMD_A_LOWER;
extern __m128i SIMD_Z_LOWER;
extern __m128i SIMD_A_UPPER;
extern __m128i SIMD_Z_UPPER;
extern __m128i SIMD_UPPERCASE_BIT;
extern __m128i SIMD_HASHTAG;
extern __m128i SIMD_SPACE;
extern __m128i SIMD_UNDERSCORE;
extern __m128i SIMD_DOUBLE_QUOTE;
extern __m128i SIMD_SINLGE_QUOTE;
extern __m128i SIMD_BACKSLASH;
extern __m128i SIMD_COMMA;
extern __m128i SIMD_SEMICOLON;
extern __m128i SIMD_NEWLINE;
extern __m128i SIMD_CLOSING_CURLY_BRACE;
extern __m128i SIMD_CLOSING_CIRCLE_BRACE;

// Constant initialization:
void simd_init(void);

// Constructors:
static inline __m128i simd_zeroInit(void) {
    return _mm_setzero_si128();
}
static inline __m128i simd_splat(char c) {
    return _mm_set1_epi8(c);
}

static inline __m128i simd_usplat(char c) {
    return _mm_xor_si128(
        _mm_set1_epi8(c),
        SIMD_SIGN_BIT
    );
}

static inline __m128i simd_load(const void* p) {
    return _mm_loadu_si128((const __m128i*)p);
}

static inline __m128i simd_uload(const void* p) {
    return _mm_xor_si128(
        _mm_loadu_si128((const __m128i*)p),
        SIMD_SIGN_BIT
    );
}


// Boolean logic:
static inline __m128i simd_not(__m128i x) {
    return _mm_xor_si128(x, SIMD_ALL_BITS);
}

static inline __m128i simd_and(__m128i a, __m128i b) {
    return _mm_and_si128(a, b);
}

static inline __m128i simd_or(__m128i a, __m128i b) {
    return _mm_or_si128(a, b);
}

static inline __m128i simd_xor(__m128i a, __m128i b) {
    return _mm_xor_si128(a, b);
}

static inline __m128i simd_andnot(__m128i a, __m128i b) {
    return _mm_andnot_si128(a, b);
}

// Signed comparison:
static inline __m128i simd_eq(__m128i a, __m128i b) {
    return _mm_cmpeq_epi8(a, b);
}

static inline __m128i simd_gt(__m128i a, __m128i b) {
    return _mm_cmpgt_epi8(a, b);
}

static inline __m128i simd_lt(__m128i a, __m128i b) {
    return _mm_cmplt_epi8(a, b);
}

static inline __m128i simd_ge(__m128i a, __m128i b) {
    return simd_not(simd_lt(a, b));
}

static inline __m128i simd_le(__m128i a, __m128i b) {
    return simd_not(simd_gt(a, b));
}

static inline __m128i simd_neq(__m128i a, __m128i b) {
    return simd_not(simd_eq(a, b));
}

// Arithmetic
static inline __m128i simd_sub(__m128i a, __m128i b) {
    return _mm_sub_epi8(a, b);
}


// Mask utilities:
static inline unsigned short simd_mask(__m128i x) {
    return (unsigned short)_mm_movemask_epi8(x);
}

static inline int simd_any(unsigned short mask) {
    return mask != 0;
}

static inline int simd_none(unsigned short mask) {
    return mask == 0;
}

static inline int simd_all(unsigned short mask) {
    return mask == 0xFFFF;
}

static inline int simd_first(unsigned short mask) {
    return __builtin_ctz((unsigned int)mask);
}

static inline int simd_last(unsigned short mask) {
    int i = 15;
    while (!(mask & (1u << i))) i--;
    return i;
}


// Common predicates:
static inline __m128i simd_is_ascii(__m128i v) {
    return simd_lt(v, SIMD_ASCII_MAX);
}

static inline __m128i simd_is_non_ascii(__m128i v) {
    return simd_ge(v, SIMD_ASCII_MAX);
}

static inline __m128i simd_is_digit(__m128i v) {
    return simd_and(
        simd_ge(v, SIMD_ZERO),
        simd_le(v, SIMD_NINE)
    );
}

static inline __m128i simd_is_lower(__m128i v) {
    return simd_and(
        simd_ge(v, simd_splat('a')),
        simd_le(v, simd_splat('z'))
    );
}

static inline __m128i simd_is_upper(__m128i v) {
    return simd_and(
        simd_ge(v, simd_splat('A')),
        simd_le(v, simd_splat('Z'))
    );
}

static inline __m128i simd_is_alpha(__m128i v) {
    return simd_is_upper(simd_or(v, SIMD_UPPERCASE_BIT));
}

static inline __m128i simd_is_alnum(__m128i v) {
    return simd_or(
        simd_is_alpha(v),
        simd_is_digit(v)
    );
}

static inline __m128i simd_is_ascii_whitespace(__m128i v) {
    return simd_lt(v, SIMD_NON_WHITESPACE);
}




#endif
