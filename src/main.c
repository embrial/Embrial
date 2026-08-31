#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "simd.h"

//int main(int argc, char *argv[]) {
//    // argc = argument count (number of arguments passed)
//    // argv = argument vector (array of strings)
//
//    printf("Program name: %s\n", argv[0]);
//    printf("Number of arguments: %d\n", argc - 1);
//
//    // Loop through and print all arguments
//    for (int i = 1; i < argc; i++) {
//        printf("Argument %d: %s\n", i, argv[i]);
//    }
//
//    return 0;
//}
typedef enum {
    TERMINAL_VAR,
    TERMINAL_CONST,
    TERMINAL_MODULE,
    TERMINAL_STRUCT,
    TERMINAL_ENUM,
    TERMINAL_INTERFACE,
    TERMINAL_IMPL,
    TERMINAL_SELF,
    TERMINAL_EXPOSE,
    TERMINAL_INCLUDE,
    TERMINAL_IMPORT,
    TERMINAL_AS,
    TERMINAL_FN,
    TERMINAL_ASSERT,
    TERMINAL_IF,
    TERMINAL_MATCH,
    TERMINAL_LOOP,
    TERMINAL_FOR,
    TERMINAL_BY,
    TERMINAL_WHILE,
    TERMINAL_BREAK,
    TERMINAL_CONTINUE,
    TERMINAL_YIELD,
    TERMINAL_RETURN,
    TERMINAL_LABEL,
    TERMINAL_COMPTIME,
    TERMINAL_TRY,
    TERMINAL_CATCH,
    TERMINAL_ORELSE,
    NONE
} GrammarSymbol;
const char *keywords[] = {
    "var\0            ",
    "const\0          ",
    "module\0         ",
    "struct\0         ",
    "enum\0           ",
    "interface\0      ",
    "impl\0           ",
    "self\0           ",
    "expose\0         ",
    "include\0        ",
    "import\0         ",
    "as\0             ",
    "fn\0             ",
    "assert\0         ",
    "if\0             ",
    "match\0          ",
    "loop\0           ",
    "for\0            ",
    "by\0             ",
    "while\0          ",
    "break\0          ",
    "continue\0       ",
    "yield\0          ",
    "return\0         ",
    "label\0          ",
    "comptime\0       ",
    "try\0            ",
    "catch\0          ",
    "orelse\0         ",
};
typedef unsigned long long U64;
U64 keywordInBase26_pack10(const char *keyword) {
    U64 result = 0;

    for (size_t i = 0; i < 10 && keyword[i]; ++i) {
        U64 digit = (U64)(keyword[i] - 'a');
        result |= digit << (i * 5);
    }

    return result;
}
bool   strCompare(const char* target, const char* actual, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (target[i] != actual[i]) return false;
	}
	return true;
}
static inline U64 packKeyword(__m128i v, size_t len) {
    v = simd_sub(v, SIMD_A_LOWER);
    U64 lo = (U64)_mm_cvtsi128_si64(v);
    U64 hi = (U64)_mm_extract_epi16(v, 4);

    U64 mask = (1ULL << (len * 5)) - 1;

    lo = _pext_u64(lo, 0x1F1F1F1F1F1F1F1FULL);
    hi = _pext_u64(hi, 0x1F1F);

    return (lo | (hi << 40)) & mask;
}

__attribute__((noinline))
GrammarSymbol dfa(const char* keyword, size_t len) {
    GrammarSymbol tokenType = NONE;
        switch (len) {
            case 2:
                switch (*keyword) {
                    case 'a':
                        if (keyword[1] == 's') tokenType = TERMINAL_AS;
                        break;
                    case 'b':
                        if (keyword[1] == 'y') tokenType = TERMINAL_BY;
                        break;
                    case 'f':
                        if (keyword[1] == 'n') tokenType = TERMINAL_FN;
                        break;
                    case 'i':
                        if (keyword[1] == 'f') tokenType = TERMINAL_IF;
                        break;
                    default:
                        break;
                }
                // "as", 
                // "by"
                // "fn", 
                // "if", 
                break;
            case 3:
                switch (keyword[0]) {
                    case 't':
                        if (keyword[1] == 'r' && keyword[2] == 'y') tokenType = TERMINAL_TRY;
                        break;
                    case 'v':
                        if (keyword[1] == 'a' && keyword[2] == 'r') tokenType = TERMINAL_VAR;
                        break;
                    case 'f':
                        if (keyword[1] == 'o' && keyword[2] == 'r') tokenType = TERMINAL_FOR;
                        break;
                    default:
                        break;
                }
                // "try",
                // "var",       
                // "for",       
                break;
            case 4:
                switch (keyword[0]) {
                    case 'e':
                        if (strCompare("num", keyword + 1, 3)) tokenType = TERMINAL_ENUM;
                        break;
                    case 's':
                        if (strCompare("elf", keyword + 1, 3)) tokenType = TERMINAL_SELF;
                        break;
                    case 'l':
                        if (strCompare("oop", keyword + 1, 3)) tokenType = TERMINAL_ENUM;
                        break;
                    case 'i':
                        if (strCompare("mpl", keyword + 1, 3)) tokenType = TERMINAL_IMPL;
                    default:
                        break;
                }
                // "enum",
                // "self", 
                // "loop", 
                // "null", 
                // "true", 
                // "impl",
                break;
            case 5:
                switch (keyword[0]) {
                    case 'b':
                        if (strCompare("reak", keyword + 1, 4)) tokenType = TERMINAL_BREAK;
                        break;
                    case 'c':
                        switch (keyword[1]) {
                            case 'a':
                                if (strCompare("tch", keyword + 2, 3)) tokenType = TERMINAL_CATCH;
                                break;
                            case 'o':
                                if (strCompare("nst", keyword + 2, 3)) tokenType = TERMINAL_CONST;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 'l':
                        if (strCompare("abel", keyword + 1, 4)) tokenType = TERMINAL_LABEL;
                        break;
                    case 'm':
                        if (strCompare("atch", keyword + 1, 4)) tokenType = TERMINAL_MATCH;
                        break;
                    case 'w':
                        if (strCompare("hile", keyword + 1, 4)) tokenType = TERMINAL_WHILE;
                        break;
                    case 'y':
                        if (strCompare("ield", keyword + 1, 4)) tokenType = TERMINAL_YIELD;
                        break;
                    default:
                        break;
                }
                // "break", 
                // "catch",
                // "class", 
                // "const", 
                // "false", 
                // "label", 
                // "match", 
                // "while", 
                // "yield", 
                break;
            case 6:
                switch (keyword[0]) {
                    case 'i':
                        if (strCompare("mport", keyword + 1, 5)) tokenType = TERMINAL_IMPORT;
                        break;
                    case 'o':
                        if (strCompare("relse", keyword + 1, 5)) tokenType = TERMINAL_ORELSE;
                        break;
                    case 'r':
                        if (strCompare("eturn", keyword + 1, 5)) tokenType = TERMINAL_RETURN;
                        break;
                    case 's':
                        if (strCompare("truct", keyword + 1, 5)) tokenType = TERMINAL_STRUCT;
                        break;
                    default:
                        break;
                }
                // "import", 
                // "public",
                // "orelse", 
                // "return", 
                // "struct", 
                break;
            case 7:
                if (strCompare("include", keyword, 7)) tokenType = TERMINAL_INCLUDE;
                break;
            case 8:
                if (keyword[0] != 'c' || keyword[1] != 'o' || keyword[7] != 'e') break;
                if (strCompare("mptim", keyword + 2, 5)) tokenType = TERMINAL_COMPTIME;
                if (strCompare("ntinu", keyword + 2, 5)) tokenType = TERMINAL_CONTINUE;
                // "comptime", 
                // "continue", 
            case 9:
                if (strCompare("interface", keyword, 9)) tokenType = TERMINAL_INTERFACE;
                // "interface", 
                break;
            default:
                break;
        }
        return tokenType;
}
__attribute__((noinline))
GrammarSymbol packed(__m128i vector, size_t len) {

    GrammarSymbol tokenType = NONE;
        U64 id = packKeyword(vector, len);
        switch (id) {
        case 17429: // var
            tokenType = TERMINAL_VAR;
            break;
        case 20526530: // const
            tokenType = TERMINAL_CONST;
            break;
        case 146410956: // module
            tokenType = TERMINAL_MODULE;
            break;
        case 414116: // enum
            tokenType = TERMINAL_ENUM;
            break;
        case 4466951736744: // interface
            tokenType = TERMINAL_INTERFACE;
            break;
        case 376200: // impl
            tokenType = TERMINAL_IMPL;
            break;
        case 175250: // self
            tokenType = TERMINAL_SELF;
            break;
        case 153566948: // expose
            tokenType = TERMINAL_EXPOSE;
            break;
        case 4416965032: // include
            tokenType = TERMINAL_INCLUDE;
            break;
        case 655834504: // import
            tokenType = TERMINAL_IMPORT;
            break;
        case 576: // as
            tokenType = TERMINAL_AS;
            break;
        case 421: // fn
            tokenType = TERMINAL_FN;
            break;
        case 655510080: // assert
            tokenType = TERMINAL_ASSERT;
            break;
        case 168: // if
            tokenType = TERMINAL_IF;
            break;
        case 7425036: // match
            tokenType = TERMINAL_MATCH;
            break;
        case 506315: // loop
            tokenType = TERMINAL_LOOP;
            break;
        case 17861: // for
            tokenType = TERMINAL_FOR;
            break;
        case 769: // by
            tokenType = TERMINAL_BY;
            break;
        case 4563190: // while
            tokenType = TERMINAL_WHILE;
            break;
        case 10490401: // break
            tokenType = TERMINAL_BREAK;
            break;
        case 159359022530: // continue
            tokenType = TERMINAL_CONTINUE;
            break;
        case 3510552: // yield
            tokenType = TERMINAL_YIELD;
            break;
        case 454708369: // return
            tokenType = TERMINAL_RETURN;
            break;
        case 11666443: // label
            tokenType = TERMINAL_LABEL;
            break;
        case 150612718018: // comptime
            tokenType = TERMINAL_COMPTIME;
            break;
        case 25139: // try
            tokenType = TERMINAL_TRY;
            break;
        case 7425026: // catch
            tokenType = TERMINAL_CATCH;
            break;
        case 153457198: // orelse
            tokenType = TERMINAL_ORELSE;
            break;
        default:
            break;
        }
    return tokenType;
}
int main(void) {
    simd_init();
    struct timespec start;
    struct timespec now;
    U64 elapsedNanoseconds;
    GrammarSymbol tokenType;
    for (int i = 0; i < 28; i++) {
        const char *keyword = keywords[i];
        size_t len = 0;
        while (keyword[len]) {
            len += 1;
        }
        tokenType = NONE;
        clock_gettime(CLOCK_MONOTONIC, &start);

        tokenType = dfa(keyword, len);
        
        
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsedNanoseconds =    (U64)(now.tv_sec - start.tv_sec) * 1000000000ull +
                                (U64)(now.tv_nsec - start.tv_nsec);

        printf("%llu\n", elapsedNanoseconds);

        __m128i vector = simd_uload(keyword);
        clock_gettime(CLOCK_MONOTONIC, &start);

        tokenType = packed(vector, len);


        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsedNanoseconds =    (U64)(now.tv_sec - start.tv_sec) * 1000000000ull +
                                (U64)(now.tv_nsec - start.tv_nsec);

        printf("%llu\n", elapsedNanoseconds);


    }
    return 0;
} 
