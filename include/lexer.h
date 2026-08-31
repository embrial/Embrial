#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#include "langType.h"
#include "lang_string.h"
#include "arena.h"
#include "mmap.h"
#include "simd.h"

// constant intitializtion:
void lexing_init(void);

// char predicates:
bool isDigit(char c);
bool isAlpha(char c);
bool isNum(char c);
bool isAlphaHex(char c);
bool isHex(char c);

// Keyword table definition:
bool KeywordTable_init(void);

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

// lexer init:
Lexer Lexer_init(const char* filepath);

// cursor management:
void Lexer_commit(Lexer* self);
void Lexer_rollback(Lexer* self);
bool Lexer_advanceCursorEnd(Lexer* self);   // returns true if cursor would move
                                            // past the end of the file

// cursor end functions:
char Lexer_currentChar(Lexer* self);    // output current char
char Lexer_advanceChar(Lexer* self);    // advance cursor end, output current char
char Lexer_consumeChar(Lexer* self);    // read current char, advance cursor end, output read char

// cursor end charachter skipping:
void Lexer_skipUntil(Lexer *self, char c);
void Lexer_skipUntilNot(Lexer *self, char c);
void Lexer_skipUntilEither(Lexer *self, char c1, char c2);
void Lexer_skipUntilSync(Lexer *self, char c);
void Lexer_skipUntilStringRelevant(Lexer *self);
void Lexer_skipWhitespace(Lexer *self);

// token scaning:
bool Lexer_scanNumber(Lexer *self);
bool Lexer_scanSymbol(Lexer *self);
bool Lexer_scanIdentifier(Lexer *self);
bool Lexer_scanKeyword(Lexer *self);

#endif
