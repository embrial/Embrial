/*
Header comment:
    File:       lexer.c
    Project:    lang
    Created:    2026-08-25

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:

Usage:

Todo: 
    Reasearch fast minimal perfect hashing, currently using brute force
    approach

*/
#include <time.h>
#include <stdio.h>
#include <errno.h>

#include "lexer.h"

// char predicates:
bool isDigit(char c) {
    return c >= '0' && c <= '9';
}
bool isAlpha(char c) {
    c |= 0x20;
    return c >= 'a' && c <= 'z';
}
bool isNum(char c) {
    return c >= '0' && c <= '9';
}
bool isAlphaHex(char c) {
    c |= 0x20;
    return c >= 'a' && c <= 'f';
}
bool isHex(char c) {
    return isDigit(c) || isAlphaHex(c);
}

// Keyword table definition:
U64 seed = 0x123456789ABCDEFULL;

typedef struct { 
    U64 capacity;
    U64 magic;
    U64 shift;
    U8 *data;
} KeywordTable;

KeywordTable keywordTable;

// SourceLocation definition:
typedef struct {
    size_t lineNumber;
    size_t lastNewline;
    size_t offset;
} SourceLocation;

// lexer type definition:

typedef enum {
    LEXER_STATUS_OK,
    LEXER_STATUS_FAILED_TO_LOAD_FILE,
    LEXER_STATUS_REACHED_EOF,
    LEXER_STATUS_UNEXPECTED_CHARACTER,
} LexerStatus;

typedef struct {
    MMap file;
    size_t lineNumber;
    size_t lastNewline;

    size_t cursorStart;
    size_t cursorEnd;
    GrammarSymbol tokenType;
    LexerStatus status;
} Lexer;

// lexer init and deninit:
Lexer Lexer_init(const char* filepath) {
    Lexer lexer = {0};
    lexer.file = MMap_file(filepath);
    if (!lexer.file.loaded) lexer.status = LEXER_STATUS_FAILED_TO_LOAD_FILE;
    return lexer;
}
void Lexer_free(Lexer* self) {
    if (self->status == LEXER_STATUS_FAILED_TO_LOAD_FILE) return;
    MMap_free(&self->file);
}

// cursor management:

void Lexer_commit(Lexer* self) {
    self->cursorStart = self->cursorEnd;
}
void Lexer_rollback(Lexer* self) {
    self->cursorEnd = self->cursorStart;
}
bool Lexer_advanceCursorEnd(Lexer* self) {
    if (self->cursorEnd + 1 >= self->file.len) {
        self->cursorEnd = self->file.len - 1;
        self->status = LEXER_STATUS_REACHED_EOF;
        return true;
    }
    self->cursorEnd += 1;
    return false;
}

// cursor end functins:
static inline char currentChar(Lexer *lexer) {
    return lexer->file.data[lexer->cursorEnd];
}
char Lexer_currentChar(Lexer* self) {
    if (self->cursorEnd >= self->file.len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return '\0';
    }
    return currentChar(self);
}
char Lexer_advanceChar(Lexer* self) {
    if (Lexer_advanceCursorEnd(self)) return '\0';
    return currentChar(self);
}
char Lexer_consumeChar(Lexer* self) {
    char c = Lexer_currentChar(self);
    if (c != '\0') self->cursorEnd += 1;
    return c;
}


// cursor end charachter skipping:
void Lexer_skipComment(Lexer *self) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    if (currentChar(self) != '#') return;
    
    while (self->cursorEnd < len) {
        v = simd_uload(MMap_at(&self->file, self->cursorEnd));

        is_newline = simd_eq(v, SIMD_NEWLINE);

        nl_mask = simd_mask(is_newline);

        // CASE 1: full block of non newlines
        if (nl_mask = 0x0000) {
        self->cursorEnd += 16;
        continue;
        }

        // CASE 2: atleast one newline
        // advance the cursor end to be on the newline
        self->cursorEnd += simd_first(nl_mask);

        // update the line information
        self->lineNumber += 1;
        self->lastNewline = self->cursorEnd;

        // consume the newline
        self->cursorEnd += 1;
    }
}
void Lexer_skipWhitespace(Lexer *self) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    if (currentChar(self) == '#') skipUntil(self, '#');
    while (self->cursorEnd < len) {
        __m128i v = simd_uload(MMap_at(&self->file, self->cursorEnd));

        __m128i is_ascii_ws = simd_is_ascii_whitespace(v);
        __m128i is_non_ascii = simd_is_non_ascii(v);

        // whitespace including newlines
        __m128i is_whitespace = simd_or(is_ascii_ws, is_non_ascii);

        // newline mask
        __m128i is_newline = simd_eq(v, SIMD_NEWLINE);

        unsigned short ws_mask = simd_mask(is_whitespace);
        unsigned short nl_mask = simd_mask(is_newline);

        // CASE 1: full block of whitespace
        if (simd_all(ws_mask)) {
            // newline record keeping
            if (nl_mask) {
                self->lineNumber += __builtin_popcount(nl_mask);
                self->lastNewline = self->cursorEnd + simd_last(nl_mask);
            }
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one non whitespace

        // first non-whitespace
        int offset = simd_first((unsigned int)~ws_mask);

        // newline handling BEFORE stop point
        unsigned short nl_before = nl_mask & ((1u << offset) - 1);


        // last newline in this prefix
        if (nl_before) {
            self->lineNumber += __builtin_popcount(nl_before);
            self->lastNewline = self->cursorEnd + simd_last(nl_before);
        }

        self->cursorEnd += offset;

        // exit white space skipping if not at the begining of a comment
        if (self->cursorEnd >= len || currentChar(self) != '#') break;

        // advance past the comment

        // consume the '#'
        self->cursorEnd += 1;

        // skip until the next newline
        while (self->cursorEnd < len) {
            v = simd_uload(MMap_at(&self->file, self->cursorEnd));

            // newline mask
            is_newline = simd_eq(v, SIMD_NEWLINE);

            nl_mask = (unsigned short)_mm_movemask_epi8(is_newline);

            // CASE 1: full block of non newlines
            if (simd_none(nl_mask)) {
                self->cursorEnd += 16;
                continue;
            }

            // CASE 2: atleast one newline
            // advance the cursor end to be on the newline
            self->cursorEnd += simd_first(nl_mask);

            // update the line information
            self->lineNumber += 1;
            self->lastNewline = self->cursorEnd;

            // consume the newline, and continue skiping whitespace
            self->cursorEnd += 1;
            break;
        }
    }
}
void Lexer_skipUntil(Lexer *self, char delim) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    __m128i splat = simd_usplat(delim);
    while (self->cursorEnd < len) {
        __m128i v = simd_uload(MMap_at(&self->file, self->cursorEnd));
        unsigned short mask = simd_mask(simd_eq(v, splat));

        // CASE 1: no matches
        if (simd_none(mask)) {
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one match

        // advance the cursor to be on the delimeter
        self->cursorEnd += simd_first(ws_mask);
        break;
    }
}
void Lexer_skipUntilNot(Lexer* self, char matching) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }

    __m128i splat = simd_usplat(matching);
    while (self->cursorEnd < len) {
        __m128i v = simd_uload(MMap_at(&self->file, self->cursorEnd));
        unsigned short mask = simd_mask(simd_neq(v, splat));

        // CASE 1: no matches
        if (simd_none(mask)) {
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one non match

        // advance the cursor to be on the first non matching charachter
        self->cursorEnd += simd_first(ws_mask);
        break;
    }
}
void Lexer_skipUntilSync(Lexer *self, char c) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    __m128i splat;
    switch (c) {
        case ',':
            splat = SIMD_COMMA;
            break;
        case ';':
            splat = SIMD_SEMICOLON;
            break;
        case ')':
            splat = SIMD_CLOSING_CIRCLE_BRACE;
            break;
        case '}':
            splat = SIMD_CLOSING_CURLY_BRACE;
            break;
        default: return;
    }
    while (self->cursorEnd < len) {
        __m128i v = simd_uload(MMap_at(&self->file, self->cursorEnd));

        __m128i is_ascii_ws = simd_is_ascii_whitespace(v);
        __m128i is_non_ascii = simd_is_non_ascii(v);

        // newline mask
        __m128i is_newline = simd_eq(v, SIMD_NEWLINE);

        unsigned short mask = simd_mask(simd_neq(simd_or(
            simd_eq(v, splat),
            simd_eq(v, SIMD_HASHTAG)
        )));
        unsigned short nl_mask = simd_mask(simd_eq(v, SIMD_NEWLINE));

        // CASE 1: full block of non syncronizing charachters or hashtags
        if (simd_all(mask)) {
            // newline record keeping
            if (nl_mask) {
                self->lineNumber += __builtin_popcount(nl_mask);
                self->lastNewline = self->cursorEnd + simd_last(nl_mask);
            }
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one non whitespace

        // syncronizing charachter or hashtag
        int offset = simd_first((unsigned int)~mask);

        // newline handling BEFORE stop point
        unsigned short nl_before = nl_mask & ((1u << offset) - 1);


        // last newline in this prefix
        if (nl_before) {
            self->lineNumber += __builtin_popcount(nl_before);
            self->lastNewline = self->cursorEnd + simd_last(nl_before);
        }

        self->cursorEnd += offset;

        // exit white space skipping if not at the begining of a comment
        if (self->cursorEnd >= len || currentChar(self) != '#') break;

        // advance past the comment

        // consume the '#'
        self->cursorEnd += 1;

        // skip until the next newline
        while (self->cursorEnd < len) {
            v = simd_uload(MMap_at(&self->file, self->cursorEnd));

            // newline mask
            is_newline = simd_eq(v, SIMD_NEWLINE);

            nl_mask = (unsigned short)_mm_movemask_epi8(is_newline);

            // CASE 1: full block of non newlines
            if (simd_none(nl_mask)) {
                self->cursorEnd += 16;
                continue;
            }

            // CASE 2: atleast one newline
            // advance the cursor end to be on the newline
            self->cursorEnd += simd_first(nl_mask);

            // update the line information
            self->lineNumber += 1;
            self->lastNewline = self->cursorEnd;

            // consume the newline, and continue skiping whitespace
            self->cursorEnd += 1;
            break;
        }
    }
}
void Lexer_skipUntilStringRelevant(Lexer *self);

static inline U64 keywordInBase26_pack10(const char *keyword) {
    U64 result = 0;

    for (size_t i = 0; i < 10 && keyword[i]; ++i) {
        U64 digit = (U64)(keyword[i] - 'a');
        result |= digit << (i * 5);
    }

    return result;
}
bool loadKeywordTable(
    KeywordTable *table,
    GrammarSymbol firstKeyword,
    size_t keywordCount
) {
    FILE *f = fopen(".LangMagicsCache", "rb");

    if (!f) {
        if (errno == ENOENT) {
            seed = 0x123456789ABCDEFULL;
            table->capacity = 1 << 50;
            table->data = malloc(table->capacity);
            return table->data;
        }

        return false; // Couldn't open for another reason.
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }

    long fileSize = ftell(f);

    if (fileSize != sizeof(U64) * (4 + keywordCount)) {
        fclose(f);
        return false;
    }

    rewind(f);

    fread(&seed, sizeof(U64), 1, f);
    fread(&table->capacity, sizeof(U64), 1, f);
    fread(&table->magic,     sizeof(U64), 1, f);
    fread(&table->shift,     sizeof(U64), 1, f);

    table->data = malloc(table->capacity);
    if (!table->data) {
        fclose(f);
        return true;
    }

    size_t lastKeyword = firstKeyword + keywordCount;
    U64 index;
    for (size_t i = firstKeyword; i < lastKeyword; i++) {
        fread(&index, sizeof(U64), 1, f);
        if (index > table->capacity) {
            fatal("corrupted .LangMagicCache file, index > capacity"); 
        }
        table->data[index] = i;
    }

    fclose(f);
    return true;
}
static inline U64 base26_pack10(U64 x) {
    U64 result = 0;

    for (int i = 0; i < 10; ++i) {
        U64 digit = x % 26;
        result |= digit << (i * 5);
        x /= 26;
    }

    return result;
}
bool KeywordTable_init(void) {
    struct timespec start;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    KeywordTable *table = &keywordTable;    // can't be bothered to refactor to
                                            // inline the function
                                            // loadKeywordTable or convert all
                                            // the -> to . instead to avoid one
                                            // level of indirection

    U64 elapsedNanoseconds;
    KeywordTable table = {0};
    if (!loadKeywordTable(&table, KEYWORD_VAR, 32)) return false;

    U64 maxIndex = 0;
    U64 minIndex = table->capacity;
    U64 packed;
    U64 magic;
    do {
        memset(table->data, TERMINAL_IDENTIFIER, table->capacity);
        magic = randomU64() & randomU64() & randomU64();
        
        size_t lastKeword = KEYWORD_VAR + 32;
        U64 packed[32] = {0};
        for (unsigned char i = KEYWORD_VAR; i < lastKeyword; i++) {
            U64 p = keywordInBase26_pack10(keywords[i]);
            index = (p * magic) >> 14;

            if (index > minIndex) goto invalidMagic;
            if (index > maxIndex) maxIndex = index;
            packed[i] = p;
            table->data[index] = i;
        }

        // 26^10 = 141167095653376
        for (U64 i = 0; i < 141167095653376; i++) {
            index = (base26_pack10(i) * magic) >> 14;

            if (index > minIndex) goto invalidMagic;
            if (table->data[i]) {
                for (int j = 0; j < 32; j++) {
                    if (i == packed[j]) goto invalidMagic;
                }
            }
            if (index > maxIndex) maxIndex = index;
        }

        if (maxIndex < minIndex) minIndex = maxIndex;
        
    invalidMagic://

        clock_gettime(clock_monotonic, &now);
        elpasednanoseconds =    (u64)(now.tv_sec - start.tv_sec) * 1000000000ull +
                                (u64)(now.tv_nsec - start.tv_nsec);
    } while (elapsedNanoseconds < 5000000000);

    U8 *temp = realloc(table->data, minIndex);
    if (!temp) return false;
    table->capacity = minIndex;
    table->magic = magic;
    table->data = temp;

    FILE *file = fopen("file.txt", "w");
    if (!file) {
        return false;
    }

    fwrite(&seed, sizeof(U64), 1, f);
    fwrite(&table->capacity, sizeof(U64), 1, f);
    fwrite(&table->magic,     sizeof(U64), 1, f);
    fwrite(&table->shift,     sizeof(U64), 1, f);

    for (int i = 0; i < 32; i++) {
        fwrite(&packed[i], sizeof(U64), 1, f);
    }

    fclose(f);

    return true;
}

// token scaning:
// recognizeHexidecimal:
static void recognizeHexidecimal(Lexer* self) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    while (self->cursorEnd < len) {
        __m128i v = simd_load(MMap_at(&self->file, self->cursorEnd));
        __m128i isDigit = simd_is_digit(v);
        __m128i lowercase = simd_or(v, SIMD_0x20);
        __m128i isAplhaHex = simd_or(simd_ugt(lowercase, SIMD_a_m1), simd_ult(lowercase, SIMD_f_p1));
        __m128i isHex = simd_or(isDigit, isAplhaHex);
        
        unsigned short mask = simd_mask(isHex);

        // CASE 1: full block of no matching charachters
        if (mask == 0xFFFF) {
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one matching charachter

        // first matching charachter
        int offset = simd_first(~mask);

        self->cursorEnd += offset;
    }
    if (self->cursorEnd >= len) self->status = LEXER_STATUS_REACHED_EOF;
}
// recognizeBinary:
static void recognizeBinary(Lexer* self) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    while (self->cursorEnd < len) {
        __m128i v = simd_load(MMap_at(&self->file, self->cursorEnd));
        __m128i isBin = simd_or(simd_eq(v, SIMD_ZERO), simd_eq(v, SIMD_ONE));
        unsigned short mask = simd_mask(isBin);

        // CASE 1: full block of no matching charachters
        if (mask == 0xFFFF) {
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one matching charachter

        // first matching charachter
        int offset = simd_first(~mask);

        self->cursorEnd += offset;
        break;
    }
    if (self->cursorEnd >= len) self->status = LEXER_STATUS_REACHED_EOF;
}
// recognizeInterger:
static void recognizeInteger(Lexer* self) {
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;
    }
    while (self->cursorEnd < len) {
        __m128i v = simd_load(MMap_at(&self->file, self->cursorEnd));
        __m128i isDigit = simd_is_digit(v);
        unsigned short mask = simd_mask(isDigit);

        // CASE 1: full block of no matching charachters
        if (mask == 0xFFFF) {
            self->cursorEnd += 16;
            continue;
        }

        // CASE 2: atleast one matching charachter

        // first matching charachter
        int offset = simd_first(~mask);

        self->cursorEnd += offset;
        break;
    }
    if (self->cursorEnd >= len) self->status = LEXER_STATUS_REACHED_EOF;
}
// scanNumber:
bool Lexer_scanNumber(Lexer *self) {
    self->cursorEnd = self->cursorStart;
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;    
    }
    // check that the first charachter is a digit
    char c = currentChar(self);
    if (c < '0' || c > '9') {
        self->status = LEXER_STATUS_UNEXPECTED_CHARACHTER;
        return;
    }

    // consume the first charachter
    self->cursorEnd += 1;
    if (self->cursorEnd >= len) return;
    bool wasZero = c == '0';
    c = currentChar(self);
    
    switch (c) {
        case 'x':
        case 'X':
            if (!wasZero) return;
            c = Lexer_consume(self);
            if (self->status != LEXER_STATUS_OK || !isHex(c)) {
                self->cursorEnd = self->cursorStart + 1;
                return;
            }
            recognizeHexidecimal(self);
            self->tokenType = TERMINAL_HEX;
            return;
        case 'b':
        case 'B':
            if (!wasZero) return;
            c = Lexer_consume(self);
            if (self->status != LEXER_STATUS_OK || (c != '0' && c != '1')) {
                self->cursorEnd = self->cursorStart + 1;
                return;
            } 
            recognizeBinary(self);
            self->tokenType = TERMINAL_BIN;
            return;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            recognizeInteger(self);
            c = Lexer_consume(self);
            if (self->status != LEXER_STATUS_OK || c != '.') {
                self->tokenType = TERMINAL_INTEGER;
                return;
            }
        case '.':
            c = Lexer_consume(self);
            if (self->status != LEXER_STATUS_OK || c < '0' || c > '9') {
                self->cursorEnd -= 1;
                self->tokenType = TERMINAL_INTEGER;
                return;
            }
            recognizeInteger(self);
            self->tokenType = TERMINAL_FLOAT;
            return;
        default://
            return;
            
    }
}
// scanSymbol:
bool Lexer_scanSymbol(Lexer *self) {
    self->cursorEnd = self->cursorStart;
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;    
    }
    char c = currentChar(self);
    self->cursorEnd += 1;
    switch (c) {
        case '\n':
            self->tokenType = TERMINAL_NEWLINE;
            break;
        case '(':
            self->tokenType = TERMINAL_LEFT_CIRCLE_BRACE;
            break;
        case ')':
            self->tokenType = TERMINAL_RIGHT_CIRCLE_BRACE;
            break;
        case '[':
            self->tokenType = TERMINAL_LEFT_SQUARE_BRACE;
            break;
        case ']':
            self->tokenType = TERMINAL_RIGHT_SQUARE_BRACE;
            break;
        case '{':
            self->tokenType = TERMINAL_LEFT_CURLY_BRACE;
            break;
        case '}':
            self->tokenType = TERMINAL_RIGHT_CIRCLE_BRACE;
            break;
        case '~':
            self->tokenType = TERMINAL_REFERENCE;
            break;
        case '@':
            self->tokenType = TERMINAL_ALLOCATION;
            break;
        case '$':
            self->tokenType = TERMINAL_REFERENCE_TO;
            break;
        case ',':
            self->tokenType = TERMINAL_COMMA;
            break;
        case ';':
            self->tokenType = TERMINAL_SEMICOLON;
            break;
        case ':':
            self->tokenType = TERMINAL_COLON;
            break;
        case '=':
            self->tokenType = TERMINAL_ASSIGNMENT;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_EQUALS;
            }
            break;
        case '/':
            self->tokenType = TERMINAL_DIVIDE;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_DIVIDE_ASSIGNING;
            }
            break;
        case '%':
            self->tokenType = TERMINAL_MODULO;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_MODULE_ASSIGNING;
            }
            break;
        case '!':
            self->tokenType = TERMINAL_NOT;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_NOT_EQUALS;
            }
            break;
        case '.':
            self->tokenType = TERMINAL_DOT;
            c = Lexer_currentChar(self);
            if (c == '.') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_EXCLUSIVE_RANGE;
                c = Lexer_currentChar(self);
                if (c == '=') {
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_INCLUSIVE_RANGE;                    break;
                }
            }
            self->tokenType = TERMINAL_DOT;
            break;
        case '&':
            self->tokenType = TERMINAL_BITWISE_AND;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_BITWISE_AND_ASSIGNING;
            } else if (c == '&') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_LOGICAL_AND;
            }
            break;
        case '|':
            self->tokenType = TERMINAL_BITWISE_OR;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_BITWISE_OR;
            } else if (c == '|') {
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_LOGICAL_OR;
            }
            break;
        // +; +=; +%; +|; +%=; +|=;
        case '+':
            self->tokenType = TERMINAL_ADD;
            c = Lexer_currentChar(self);
            switch (c) {
                case '=':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_ADD_ASSIGNING;
                    break;
                case '%':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_ADD_WRAPPING;
                    c = Lexer_currentChar(self);
                    if (c == '=') {
                        self->cursorEnd += 1;
                        self->tokenType = TERMINAL_ADD_WRAPPING;
                    }
                    break;
                case '|':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_ADD_SATURATING;
                    c = Lexer_currentChar(self);
                    if (c == '=') {
                        self->cursorEnd += 1;
                        self->tokenType = TERMINAL_ADD_SATURATING;
                    }
                    break;
                default: break;
            }
            break;
        // -; -=; -%; -|; -%=; -|=;
        case '-':
            self->tokenType = TERMINAL_SUBTRACT;
            c = Lexer_currentChar(self);
            switch (c) {
                case '=':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_SUBTRACT;
                    break;
                case '%':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_SUBTRACT_WRAPPING;
                    c = Lexer_currentChar(self);
                    if (c == '=') {
                        self->cursorEnd += 1;
                        self->tokenType = TERMINAL_SUBTRACT_WRAPPING_ASSIGNING;
                    }
                    break;
                case '|':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_SUBTRACT_SATURATING;
                    c = Lexer_currentChar(self);
                    if (c == '=') {
                        self->cursorEnd += 1;
                        self->tokenType = TERMINAL_SUBTRACT_SATURATING_ASSIGNING;
                    }
                    break;
                default: break;
            }
            break;

        // *; *=; *%; *|; *%=; *|=;
        case '*':
            self->tokenType = TERMINAL_MULTIPLY;
            c = Lexer_currentChar(self);
            switch (c) {
                case '=':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_MULTIPLY_ASSIGNING;
                    break;
                case '%':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_MULTIPLY_WRAPPING;
                    c = Lexer_currentChar(self);
                    if (c == '=') {
                        self->cursorEnd += 1;
                        self->tokenType = TERMINAL_MULTIPLY_WRAPPING_ASSIGNING;
                    }
                    break;
                case '|':
                    self->cursorEnd += 1;
                    self->tokenType = TERMINAL_MULTIPLY_SATURATING;
                    c = Lexer_currentChar(self);
                    if (c == '=') {
                        self->cursorEnd += 1;
                        self->tokenType = TERMINAL_MULTIPLY_SATURATING_ASSIGNING;
                    }
                    break;
                default: break;
            }
            break;
        // <; <=; <<; <<=; <<|; <<|=;
        case '<':
            self->tokenType = TERMINAL_LESS_THAN;
            c = Lexer_currentChar(self);

            // <
            if (c == '=') {
                // <=
                self->cursorEnd += 1;
                self->tokenType = TERMINAL_LESS_THAN_OR_EQUAL_TO;
                break;
            }
            
            if (c != '<') break;
            self->cursorEnd += 1;

            // <<

            self->tokenType = TERMINAL_LEFT_BITSHIFT;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                // <<=
                self->tokenType = TERMINAL_RIGHT_BITSHIFT_ASSIGNING;
                break;
            }

            if (c != '|') break;
            self->cursorEnd += 1;

            // <<|

            self->tokenType = TERMINAL_LEFT_BITSHIFT_SATURATING;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                // <<|=

                self->tokenType = TERMINAL_LEFT_BITSHIFT_SATURATING_ASSIGNING;
            }
            break;
        // >; >=; >>=; >>|; >>|=;
        case '>':

            // >
            self->tokenType = TERMINAL_GREATER_THAN;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;

                // >=
                self->tokenType = TERMINAL_GREATER_THAN_OR_EQUAL_TO;
                break;
            }

            if (c != '>') break;
            self->cursorEnd += 1;
            // >>

            self->tokenType = TERMINAL_RIGHT_BITSHIFT;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                // >>=

                self->tokenType = TERMINAL_RIGHT_BITSHIFT_ASSIGNING;
                break;
            }

            if (c != '|') break;
            self->cursorEnd += 1;
            // >>|

            self->tokenType = TERMINAL_RIGHT_BITSHIFT_SATURATING;
            c = Lexer_currentChar(self);
            if (c == '=') {
                self->cursorEnd += 1;
                // >>|=

                self->tokenType = TERMINAL_RIGHT_BITSHIFT_SATURATING_ASSIGNING;
            }
            break;
        default://
            self->status = LEXER_STATUS_UNEXPECTED_CHARACHTER;
            break;
    }
}
// scanIdentifier:
bool Lexer_scanIdentifier(Lexer *self) {
    Lexer_rollback(self);
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;    
    }
    char c = currentChar(self);
    if (c == '_') {
        lexer->cursorEnd += 1;
        lexer->tokenType = TERMINAL_IDENTIFIER;
        return true;
    }
    if (!isAlpha(c)) return false;
    self->tokenType = TERMINAL_IDENTIFIER;

    __m128i v = simd_initZero();
    __m128i isDigit = simd_initZero();
    __m128i isUppercase = simd_initZero();
    __m128i isUnderscore = simd_initZero();
    while (self->cursorEnd < len) {
        v = simd_uload(MMap_at(&self->file, self->cursorEnd));
        isDigit    = simd_is_digit(v);
        isUppercase = simd_is_upper(v);
        isUnderscore = simd_eq(v, SIMD_UNDERSCORE);
        __m128i isAlphaNum = simd_or(simd_or(simd_is_lower(v), isUppercase), isDigit);
        __m128i isIdentifier = simd_or(isUnderscore, isAlphaNum);
        unsigned short mask = simd_mask(isIdentifier);

        // All 16 are valid charachters
        if (mask == 0xFFFF) {
            self->cursorEnd += 16;
            continue;
        }

        // Atleast 1 invalid charachter

        // first matching charachter
        int offset = simd_first(~mask);

        self->cursorEnd += offset;
        break;
    }
    if (self->cursorEnd >= len) self->status = LEXER_STATUS_REACHED_EOF;
}
// scanKeyword:
bool Lexer_scanKeyword(Lexer *self) {
    Lexer_rollback(self);
    size_t len = self->file.len;
    if (self->cursorEnd >= len) {
        self->status = LEXER_STATUS_REACHED_EOF;
        return;    
    }
    char c = currentChar(lexer);
    if (c == '_' || !isAlpha(c)) {
        return false;
    }

    __m128i v = simd_initZero();
    __m128i isDigit = simd_initZero();
    __m128i isUppercase = simd_initZero();
    __m128i isUnderscore = simd_initZero();
    while (self->cursorEnd < len) {
        v = simd_uload(MMap_at(&self->file, self->cursorEnd));
        isDigit    = simd_is_digit(v);
        isUppercase = simd_is_upper(v);
        isUnderscore = simd_eq(v, SIMD_UNDERSCORE);
        __m128i isAlphaNum = simd_or(simd_or(simd_is_lower(v), isUppercase), isDigit);
        __m128i isIdentifier = simd_or(isUnderscore, isAlphaNum);
        unsigned short mask = simd_mask(isIdentifier);

        // All 16 are valid charachters
        if (mask == 0xFFFF) {
            self->cursorEnd += 16;
            continue;
        }

        // Atleast 1 invalid charachter

        // first matching charachter
        int offset = simd_first(~mask);

        self->cursorEnd += offset;
        break;
    }
    if (self->cursorEnd >= len) self->status = LEXER_STATUS_REACHED_EOF;


    len = self->cursorEnd - self->cursorStart;
    if (len < 2 || len > 9 || isDigit || isUppercase || isUnderscore) 
        return false;

    // since len <= 9, the simd_uload must have only ran once
    // therefore v now contains the current identifier, the additional checks
    // in the above ensure that v only contains lowercase letters in the first
    // 0..len bytes
    U64 index = (packKeyword(v, len) * keywordTable.magic) >> 14;
    self->tokenType = keywordTable->data[index];
    return self->tokenType != TERMINAL_IDENTIFIER;
}
