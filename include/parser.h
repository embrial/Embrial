#ifndef PARSER_H
#define PARSER_H

// parser state enum:
typedef enum {
    PARSER_STATUS_UNINITIALIZED,
    PARSER_STATUS_OK,
    PARSER_STATUS_OUT_OF_MEMORY,
    PARSER_STATUS_OVERFLOW
} ParserStatus;

typedef struct Parser Parser;

typedef void (*ParseFunction)(
    Parser* parser,
    Lexer* lexer,
    size_t node
);

typedef struct {
    ParseFunction parse;
    size_t node;
} NonTerminal;

typedef Arena StringArena;
typedef Arena AstNodeArena;
typedef Arena NonTerminalArena;
typedef Arena SymbolArena;
typedef Arena SymbolTableArena;

typedef struct {
    SymbolArena symbols;
    StringArena literals;
    AstNodeArena ast;

    NonTerminalArena parseStack;
    SymbolTableArena symbolTableStack

    ParserStatus status;
} Parser;

#endif
