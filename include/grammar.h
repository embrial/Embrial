#ifndef GRAMMAR_H
#define GRAMMAR_H

/*
Header comment:
    File:       grammar.h
    Project:    lang
    Created:    2026-08-19

    Copyright (C) 2026 Adrian Long
    Licensed under the MIT License.

Description:
    grammar.h defines all of the symbols recognized by the lang compiler.

*/

// Constants:

const char *keywords[] = {
    "none",
    "var",      //  0
    "const",    //  1
    "module",   //  2
    "struct",   //  3
    "enum",     //  4
    "interface",//  5
    "impl",     //  6
    "self",     //  7
    "expose",   //  8
    "include",  //  9
    "import",   // 10
    "as",       // 11
    "fn",       // 12
    "assert",   // 13
    "if",       // 14
    "match",    // 15
    "loop",     // 16
    "for",      // 17
    "by",       // 18
    "while",    // 19
    "break",    // 20
    "continue", // 21
    "yield",    // 22
    "return",   // 23
    "label",    // 24
    "comptime", // 25
    "try",      // 26
    "catch",    // 27
    "orelse",   // 28
    "true",     // 29
    "false",    // 30
    "null",     // 31
}

typedef enum {
    GRAMMAR_SYMBOL_NONE,                            
    KEYWORD_VAR,
    KEYWORD_CONST,
    KEYWORD_MODULE,
    KEYWORD_STRUCT,
    KEYWORD_ENUM,
    KEYWORD_INTERFACE,
    KEYWORD_IMPL,
    KEYWORD_SELF,
    KEYWORD_EXPOSE,
    KEYWORD_INCLUDE,
    KEYWORD_IMPORT,
    KEYWORD_AS,
    KEYWORD_FN,
    KEYWORD_ASSERT,
    KEYWORD_IF,
    KEYWORD_MATCH,
    KEYWORD_LOOP,
    KEYWORD_FOR,
    KEYWORD_BY,
    KEYWORD_WHILE,
    KEYWORD_BREAK,
    KEYWORD_CONTINUE,
    KEYWORD_YIELD,
    KEYWORD_RETURN,
    KEYWORD_LABEL,
    KEYWORD_COMPTIME,
    KEYWORD_TRY,
    KEYWORD_CATCH,
    KEYWORD_ORELSE,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_NULL,
    TERMINAL_ADD,                                   // +
    TERMINAL_ADD_WRAPPING,                          // +%
    TERMINAL_ADD_SATURATING,                        // +|
    TERMINAL_ADD_ASSIGNING,                         // +=
    TERMINAL_ADD_WRAPPING_ASSIGNING,                // +%=
    TERMINAL_ADD_STATURATING_ASSIGNING,             // +|=
    TERMINAL_SUBTRACT,                              // -
    TERMINAL_SUBTRACT_WRAPPING,                     // -%
    TERMINAL_SUBTRACT_SATURATING,                   // -|
    TERMINAL_SUBTRACT_ASSIGNING,                    // -=
    TERMINAL_SUBTRACT_WRAPPING_ASSIGNING,           // -%=
    TERMINAL_SUBTRACT_SATURATING_ASSIGNING,         // -|=
    TERMINAL_MULTIPLY,                              // *
    TERMINAL_MULTIPLY_WRAPPING,                     // *%
    TERMINAL_MULTIPLY_SATURATING,                   // *|
    TERMINAL_MULTIPLY_ASSIGNING,                    // *=
    TERMINAL_MULTIPLY_WRAPPING_ASSIGNING,           // *%=
    TERMINAL_MULTIPLY_SATURATING_ASSIGNING,         // *|=
    TERMINAL_DIVIDE,                                // /
    TERMINAL_DIVIDE_ASSIGNING,                      // /=
    TERMINAL_MODULO,                                // %
    TERMINAL_MODULE_ASSIGNING,                      // %=
    TERMINAL_LEFT_BITSHIFT,                         // <<
    TERMINAL_LEFT_BITSHIFT_SATURATING,              // <<|
    TERMINAL_LEFT_BITSHIFT_ASSIGNING,               // <<=
    TERMINAL_LEFT_BITSHIFT_SATURATING_ASSIGNING,    // <<|=
    TERMINAL_RIGHT_BITSHIFT,                        // >>
    TERMINAL_RIGHT_BITSHIFT_SATURATING,             // >>|
    TERMINAL_RIGHT_BITSHIFT_ASSIGNING,              // >>=
    TERMINAL_RIGHT_BITSHIFT_SATURATING_ASSIGNING,   // >>|=
    TERMINAL_BITWISE_AND,                           // &
    TERMINAL_BITWISE_AND_ASSIGNING,                 // &=
    TERMINAL_BITWISE_OR,                            // |
    TERMINAL_BITWISE_OR_ASSIGNING,                  // |=
    TERMINAL_BITWISE_XOR,                           // ^
    TERMINAL_BITWISE_XOR_ASSIGNING,                 // ^=
    TERMINAL_ASSIGNMENT,                            // =
    TERMINAL_EQUALS,                                // ==
    TERMINAL_GREATER_THAN,                          // >
    TERMINAL_GREATER_THAN_OR_EQUAL_TO,              // >=
    TERMINAL_LESS_THAN,                             // <
    TERMINAL_LESS_THAN_OR_EQUAL_TO,                 // <=
    TERMINAL_NOT,                                   // !
    TERMINAL_NOT_EQUALS,                            // !=
    TERMINAL_LOGICAL_OR,                            // ||
    TERMINAL_LOGICAL_AND,                           // &&
    TERMINAL_DOT,                                   // .
    TERMINAL_EXCLUSIVE_RANGE,                       // ..
    TERMINAL_INCLUSIVE_RANGE,                       // ..=
    TERMINAL_REFERENCE,                             // ~
    TERMINAL_REFERENCE_TO,                          // $
    TERMINAL_ALLOCATION,                            // @
    TERMINAL_COLON,                                 // :
    TERMINAL_NEWLINE,                               // \n
    TERMINAL_SEMICOLON,                             // ;
    TERMINAL_COMMA,                                 // ,
    TERMINAL_LEFT_CIRCLE_BRACE,                     // (
    TERMINAL_RIGHT_CIRCLE_BRACE,                    // )
    TERMINAL_LEFT_SQUARE_BRACE,                     // [
    TERMINAL_RIGHT_SQUARE_BRACE,                    // ]
    TERMINAL_LEFT_CURLY_BRACE,                      // {
    TERMINAL_RIGHT_CURLY_BRACE,                     // }
    TERMINAL_UNDERSCORE,                            // _
    TERMINAL_INTEGER,
    TERMINAL_HEX,
    TERMINAL_BIN,
    TERMINAL_FLOAT,
    TERMINAL_STRING,
    TERMINAL_IDENTIFIER,
    NONTERMINAL_COMP_TARGET,
    NONTERMINAL_IMPORT,
} GrammarSymbol;


#endif
