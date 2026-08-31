#include "simd.h"

__m128i SIMD_ALL_BITS;
__m128i SIMD_SIGN_BIT;
__m128i SIMD_ASCII_MAX;
__m128i SIMD_NON_WHITESPACE;
__m128i SIMD_ZERO;
__m128i SIMD_NINE;
__m128i SIMD_A_LOWER;
__m128i SIMD_Z_LOWER;
__m128i SIMD_A_UPPER;
__m128i SIMD_Z_UPPER;
__m128i SIMD_UPPERCASE_BIT;
__m128i SIMD_HASHTAG;
__m128i SIMD_SPACE;
__m128i SIMD_UNDERSCORE;
__m128i SIMD_DOUBLE_QUOTE;
__m128i SIMD_SINLGE_QUOTE;
__m128i SIMD_BACKSLASH;
__m128i SIMD_COMMA;
__m128i SIMD_SEMICOLON;
__m128i SIMD_NEWLINE;
__m128i SIMD_CLOSING_CURLY_BRACE;
__m128i SIMD_CLOSING_CIRCLE_BRACE;

void simd_init(void) {
    SIMD_ALL_BITS               = simd_usplat((char)0xFF);
    SIMD_SIGN_BIT               = simd_usplat((char)0x80);
    SIMD_ASCII_MAX              = simd_usplat((char)0x80);
    SIMD_NON_WHITESPACE         = simd_usplat(0x21);
    SIMD_ZERO                   = simd_usplat('0');
    SIMD_NINE                   = simd_usplat('9');
    SIMD_A_LOWER                = simd_usplat('a');
    SIMD_Z_LOWER                = simd_usplat('z');
    SIMD_A_UPPER                = simd_usplat('A');
    SIMD_Z_UPPER                = simd_usplat('Z');
    SIMD_UPPERCASE_BIT          = simd_usplat(0x20);
    SIMD_HASHTAG                = simd_usplat('#');
    SIMD_SPACE                  = simd_usplat(' ');
    SIMD_UNDERSCPRE             = simd_usplat('_');
    SIMD_DOUBLE_QUOTE           = simd_usplat('"');
    SIMD_SINLGE_QUOTE           = simd_usplat('\'');
    SIMD_BACKSLASH              = simd_usplat('\\');
    SIMD_COMMA                = simd_usplat(',');
    SIMD_SEMICOLON            = simd_usplat(';');
    SIMD_NEWLINE              = simd_usplat('\n');
    SIMD_CLOSING_CURLY_BRACE  = simd_usplat('}');
    SIMD_CLOSING_CIRCLE_BRACE = simd_usplat(')');
}
